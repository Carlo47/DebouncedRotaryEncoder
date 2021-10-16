/**
 * Class        RotaryEncoder.cpp
 * Author       2021-10-09 Charles Geiser (https://www.dodeka.ch)
 * 
 * Purpose      Rotary encoder class with debouncing of the rotary motion 
 *              and the axial push button clicks. Two debouncing methods for
 *              the rotary encoder are implemented: 
 *              - Debouncing by cleaning of clock and data signal
 *              - Debouncing by table lookup of valid transitions
 * 
 *              For further improvment connect 22 nF capacitors from input pins
 *              to ground. With this arrangement rotational speeds of 500 rpm 
 *              ( = 10000 steps/min = 167 steps/sec) can be achieved with both 
 *              debouncing methods.  
 * 
 * Remarks      No interrupts are used. Call RotaryEncoder::loop() inside your main loop()
 * 
 * Debouncing   Debouncing by cleaning of clock and data signal
 * method 1      
 *                    ______          ______  
 *              ____||      ||______||         clock       
 *                  :       :       :
 *                 ______    :     ______
 *              _|| :    ||______|| :    ||_   data
 *               !  :    !  :    !  :    !
 *               !  :_______:    !  :_______
 *              ____|       |_______|          cleaned clock (copy stable data value on jittering clock edge)
 *               !       !       !       !
 *              _         _______         __
 *               |_______|       |_______|     cleaned data (copy stable clock value on jittering data edge)
 * 
 *              ---> Clockwise rotation when rising edge of clean clock and clean data is low  
 *              <--- Counterclockwise rotation when rising edge of clean data and clean clock is low  
 * 
 * Reference    http://www.technoblogy.com/show?1YHJ
 *              In contrast to this publication I clean both clock and data signals because
 *              bouncing not only occurs at closing or opening the contacts but also on sliding
 *              when the contacts are closed. 
 * 
 * 
 * Debouncing   Debouncing by table lookup of valid transitions
 * method 2                               T1    T2    T3    T4 
 *              Valid transitions CW   11 -> 10 -> 00 -> 01 -> 11
 *              Valid transitions CCW  11 -> 01 -> 00 -> 10 -> 11  
 *                                        t1    t2    t3    t4
 *              
 *              All possible transitions:      0000  x          0
 *              (x 0 invalid transition)       0001 CW  T3      1
 *                                             0010 CCW t3      1
 *                                             0011  x          0
 *                                             0100 CCW t2      1
 *                                             0101  x          0
 *                                             0110  x          0
 *                                             0111 CW  T4      1
 *                                             1000 CW  T2      1
 *                                             1001  x          0
 *                                             1010  x          0
 *                                             1011 CCW t4      1
 *                                             1100  x          0
 *                                             1101 CCW t1      1
 *                                             1110 CW  T1      1
 *                                             1111  x          0
 * 
 * Reference    https://www.best-microcontroller-projects.com/rotary-encoder.html
 */     

#include "RotaryEncoder.h"

/**
 * Debounce rotary encoder by cleaning clock and data signal
 */
void RotaryEncoder::_debounceRotaryByCleaning()
{
  _clkState  = digitalRead(_pinClk);
  _dataState = digitalRead(_pinData);

  if (_prevClkState != _clkState)     // clock transition detected (even bouncing)
  {
    _prevClkState = _clkState;
    _cleanedClkState = _dataState;    // copy data state to get clean clock
    digitalWrite(GPIO_NUM_2, _cleanedClkState);
  }

  if (_prevDataState != _dataState)   // data transition detected (even bouncing)
  {
    _prevDataState = _dataState;
    _cleanedDataState = _clkState;    // copy clock state to get clean data
    digitalWrite(GPIO_NUM_4, _cleanedDataState);
  }
  bool risingClk  = _prevCleanedClkState  == LOW  && _cleanedClkState == HIGH;
  bool risingData = _prevCleanedDataState == LOW  && _cleanedDataState == HIGH;

  if (risingClk  && _cleanedDataState == LOW) {_onCW();  /* Serial.printf("%s\n", "debounced by cleaning"); */}
  if (risingData && _cleanedClkState  == LOW) {_onCCW(); /* Serial.printf("%s\n", "debounced by cleaning"); */}

  _prevCleanedClkState  = _cleanedClkState;
  _prevCleanedDataState = _cleanedDataState;
} 

/**
 * Debounce rotary encoder by table lookup and handle
 *    onCW()
 *    onCCW()
 */
void RotaryEncoder::_debounceRotaryByTable()
{
  _newTransition <<= 2; // shift previous transition 2 bits to the left
  if (digitalRead(_pinClk))   _newTransition |= 0b0010;  // compose newTransition 
  if (digitalRead(_pinData))  _newTransition |= 0b0001;  // from clock and data
  _newTransition &= 0b1111;  // clear high byte, newTransition is now the index into the valid transistion table

   if (_validTransitions[_newTransition] ) 
   {
      _transitions <<= 4;  // shift old indices to the left
      _transitions |= _newTransition;   // add new transition
      if ((_transitions & 0xff) == 0b00010111) {_onCW();  /* Serial.printf("%s\n", "debounced by table"); */}  // full step in clockwise direction done (T3T4) 
      if ((_transitions & 0xff) == 0b00101011) {_onCCW(); /* Serial.printf("%s\n", "debounced by table"); */}  // full step in counterclockwise direction done (t3t4)
   }
}

/**
 * Set debouncing method
 * true  = by table lookup (this is the default method)
 * false = by cleaning clock and data signal
 */
void RotaryEncoder::setDebouncingRotEncByTable(bool byTable)
{
  _debouncingRotEncByTable = byTable;
}

/**
 * Debounce the pushbutton and handle
 *    onclick()
 *    onLongClick()
 *    onDoubleClick()
 */
void RotaryEncoder::_debounceButton()
{
  _prevButtonState = _buttonState;
  _buttonState = digitalRead(_pinButton);

  // Debouncing pushbutton
  if (_prevButtonState == HIGH && _buttonState == LOW) // Axial pushbutton pressed
  {
    _msButtonDown = millis();                          // Memorize time
  }
  else if (_prevButtonState == LOW && _buttonState == HIGH) // Pushbutton released
  {
    if (millis() - _msButtonDown < _msDebounce)        // Pushutton bounces
    {
      // Ignore bouncing
    }
    else if (millis() - _msButtonDown > _msLongClick)  // Its a long click
    {
      _onLongClick();
    }
    else
    {
      _clickCount++;                  
      if (_clickCount == 1)           // Time of 1st click, just memorize
        _msFirstClick = millis();
    }
  }
  else       // This branch only passed through when nothing is to do in loop 
  {
    if (_clickCount == 1 && millis() - _msFirstClick > _msDoubleClickGap) // Time after 1st click expired
      {
        _msFirstClick = 0;
        _clickCount = 0;
        _onClick();
      }
    else if (_clickCount > 1)         // More than 1 click done 
    {
      _msFirstClick = 0;
      _clickCount = 0;
      _onDoubleClick(); 
    } 
  }
}

/**
 * Call this method in your main loop
 */
void RotaryEncoder::loop()
{
  _debounceButton(); 
  _debouncingRotEncByTable ? _debounceRotaryByTable() : _debounceRotaryByCleaning(); 
}

// Methods to add the callbacks
void RotaryEncoder::addOnClickCB(CallbackFunction cb)
{
  _onClick = cb;
};

// 3 callbacks for pushbutton
void RotaryEncoder::addOnLongClickCB(CallbackFunction cb)
{
  _onLongClick = cb;
};

void RotaryEncoder::addOnDoubleClickCB(CallbackFunction cb)
{
  _onDoubleClick = cb;
};

// 2 callbacks for rotary encoder
void RotaryEncoder::addOnClockwiseCB(CallbackFunction cb)
{
  _onCW = cb;
};

void RotaryEncoder::addOnCounterClockwiseCB(CallbackFunction cb)
{
  _onCCW = cb;
};
