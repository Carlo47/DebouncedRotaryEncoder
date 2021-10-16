/**
 * Header       RotaryEncoder.h
 * Author       2021-10-09 Charles Geiser (https://www.dodeka.ch) 
 * 
 * Purpose      Rotary encoder class with debouncing of the rotary motion 
 *              and the axial push button clicks
 * 
 * Constructor
 * arguments    pinClk     input pin clock
 *              pinData    input pin data
 *              pinButton  input pin pushbutton (optional, only for encoders with pushbutton)
 * 
 * Remarks       Add corrresponding callbacks to be called on following actions:
 *               onCW()          Clock wise rotation
 *               onCCW()         Counter clock wise
 *               onClick()       Actuating the axial pushbutton
 *               onLongClick()   Long actuation of the axial pushbutton
 *               onDoubleClick() Double actuation of the axial pushbutton
 * 
 *               No interrupts are used. Call RotaryEncoder::loop() inside your main loop()
 */  
#ifndef _ROTARYENCODER_H_
#define _ROTARYENCODER_H_
#include <Arduino.h>

typedef void (*CallbackFunction)();

class RotaryEncoder
{
  public:
    // Encoders without pushbutton
    RotaryEncoder(uint8_t pinClk, uint8_t pinData) :
      _pinClk(pinClk), 
      _pinData(pinData)
      {
      pinMode(_pinClk, INPUT_PULLUP);
      pinMode(_pinData, INPUT_PULLUP);
      }

    // Encoders with axial pushbutton
    RotaryEncoder(uint8_t pinClk, uint8_t pinData, uint8_t pinButton) : 
      _pinClk(pinClk), 
      _pinData(pinData), 
      _pinButton(pinButton)
    {
      pinMode(_pinClk, INPUT_PULLUP);
      pinMode(_pinData, INPUT_PULLUP);
      pinMode(_pinButton, INPUT_PULLUP);
    }
 
    void setDebouncingRotEncByTable(bool byTable = true);  // byTable=false selects debouncing by cleaning clock and data signal
    void addOnClickCB(CallbackFunction cb);
    void addOnLongClickCB(CallbackFunction cb);
    void addOnDoubleClickCB(CallbackFunction cb);
    void addOnClockwiseCB(CallbackFunction cb);
    void addOnCounterClockwiseCB(CallbackFunction cb);

    void loop();
   
  private:
    static void _nop(){};
    void _debounceRotaryByCleaning();
    void _debounceRotaryByTable();
    void _debounceButton();
    CallbackFunction _onClick = _nop;
    CallbackFunction _onLongClick = _nop;
    CallbackFunction _onDoubleClick = _nop;
    CallbackFunction _onCW = _nop;
    CallbackFunction _onCCW = _nop;
    uint8_t _clkState = HIGH;
    uint8_t _prevClkState = LOW;
    uint8_t _cleanedClkState;
    uint8_t _prevCleanedClkState = HIGH;
    uint8_t _buttonState = HIGH;
    uint8_t _prevButtonState;
    uint8_t _dataState = HIGH;
    uint8_t _prevDataState;
    uint8_t _cleanedDataState;
    uint8_t _prevCleanedDataState;
    uint8_t _pinClk;
    uint8_t _pinData;
    uint8_t _pinButton;
    uint8_t _clickCount = 0;
    unsigned long _msDebounce = 50;        // After 50ms the button should have reached a stationary state
    unsigned long _msLongClick = 300;      // Button held longer than 300ms is considered LongClick
    unsigned long _msDoubleClickGap = 250; // Two button clicks within 250ms count as DoubleClick
    unsigned long _msButtonDown;
    unsigned long _msFirstClick = 0;
    uint8_t _newTransition = 0;
    uint16_t _transitions = 0;
    const uint8_t _validTransitions[16] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
    bool _debouncingRotEncByTable = true;
};
#endif