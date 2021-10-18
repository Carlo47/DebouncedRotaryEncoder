# DebouncedRotaryEncoder
Debounces the noisy steps of a rotary encoder and implements callbacks for the 
events onclockwise and oncounterclockwise rotation.

Analogous to the "debounced pushbutton", the RotaryEncoder class debounces the rotary 
motion of an encoder. The user implements his actions to be executed on forward or 
backward rotation and registers them with the encoder with `addOnClockwiseCB()` and 
`addOnCounterClockwiseCB()`.

The test program increments a counter on clockwise rotation or decrements it on 
counterclockwise rotation. No pulses should be lost or added regardless of the 
rotation speed. 

By hand, with best effort, one revolution (20 steps) could be executed in 70 ms, which 
corresponds to a rotation speed of 60000/70 = 857 rpm = 14.3 rps = 286 steps/second.

The axial pushbutton can be used to switch between two different debouncing methods 
or to query the angular position of the encoder.
