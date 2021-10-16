/**
 * Program      testRotaryEncoder.cpp
 * Author       2021-10-14 Charles Geiser (https://www.dodeka.ch)
 * 
 * Purpose      Test of 2 debouncing methods for rotary encoders.
 *              A counter is incremented on clockwise rotation or decremented on 
 *              counterclockwise rotation. No pulses should be lost or added regardless 
 *              of the rotation speed. 
 *              By hand, with best effort, one revolution (20 steps) could be executed in 70 ms, which 
 *              corresponds to a rotation speed of 60000/70 = 857 rpm = 14.3 rps = 286 steps/second.
 * 
 *              The following callback functions are implemented:
 *              - onCW()           Increment counter on each step in clockwise direction
 *              - onCCW()          Decrement counter on each step in counterclockwise direction
 *              - onClick()        Reset counter and select debouncing method by table lookup
 *              - onLongClick()    Reset counter and select debouncing method by cleaning of clock and data signal  
 *              - onDoubleClick()  Show angular position of rotary encoder
 * 
 * Board        ESP32 DoIt DevKit V1
 *
 * Wiring
 *                          USB                                   
 *                   .------I I------.                    
 *                  -|3V3   ```   Vin|-                  
 *                  -|GND         GND|----+------+------+            
 *                  -|D15  ESP32  D13|-   | 22nF |      |        
 *                  -|D2   DevKit D12|-  ===    ===     |         
 *                  -|D4     V1   D14|-   |      |      |           .----------------.  
 *                  -|RX2         D27|----|------+------|-----------| CLK            |       
 *                  -|TX2         D26|----+-------------|-----------| DT    Rotary   |            
 *                  -|D5          D25|------------------|-----------| SW    Encoder  | 
 *                  -|D18         D33|-                 |  3.3V <---| +      with    | 
 *                  -|D19         D32|-                 +-----------| GND  Pusbutton |     
 *                  -|D21         D35|-                             `----------------´    
 *                  -|RX0         D34|-             
 *                  -|TX0          VN|-                             
 *                  -|D22          VP|-                           
 *                  -|D23          EN|-                   
 *                   `---------------´                       
 *
 * Remarks      Call RotaryEncoder::loop() inside your main loop()
 *
 * References   http://www.technoblogy.com/show?1YHJ
 *              https://www.best-microcontroller-projects.com/rotary-encoder.html           
 */
#include "RotaryEncoder.h"

const uint8_t PIN_CTRLKNOB_SW  = GPIO_NUM_25;
const uint8_t PIN_CTRLKNOB_DAT = GPIO_NUM_26;
const uint8_t PIN_CTRLKNOB_CLK = GPIO_NUM_27;

RotaryEncoder ctrlKnob(PIN_CTRLKNOB_CLK, PIN_CTRLKNOB_DAT, PIN_CTRLKNOB_SW);
int counter = 0;

/**
 * Reset the counter and select debouncing method "table lookup of valid transitions"
 */
void onClick()
{
  counter = 0;
  ctrlKnob.setDebouncingRotEncByTable();
  Serial.printf("Debouncing by table lookup, counter set to %d\n", counter);
}

/**
 * Reset counter and select debouncing method "cleaning of clock and data signals"
 */
void onLongClick()
{
  counter = 0;
  ctrlKnob.setDebouncingRotEncByTable(false);
  Serial.printf("Debouncing by cleaning of clock and data signals, counter set to %d\n", counter);
}

/**
 * Show angular position of rotary encoder
 * 1 step = 18° (20 steps per revolution)
 */
void onDoubleClick()
{
  Serial.printf("Position = %d°\n", (18 * counter) % 360);
}

/**
 * Callback which is called on every step in clockwise direction
 */
void countUp()
{
  counter++;
  Serial.printf("count = %4d\n", counter);
}

/**
 * Callback which is called on every step in counterclockwise direction
 */
void countDown()
{
  counter--;
  Serial.printf("count = %4d\n", counter);
}


void setup() 
{
  Serial.begin(115200);

  // Add the callbacks
  ctrlKnob.addOnClickCB(onClick);
  ctrlKnob.addOnLongClickCB(onLongClick);
  ctrlKnob.addOnDoubleClickCB(onDoubleClick);
  ctrlKnob.addOnClockwiseCB(countUp);
  ctrlKnob.addOnCounterClockwiseCB(countDown);
}

void loop() 
{
  ctrlKnob.loop();
}