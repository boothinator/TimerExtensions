# ArduinoTimerExtensions

/Easy access to advanced Arduino/AVR timer functions, such as precise timing, pulse generation, and input event capture./

## Features

* Abstracts much of the Arduino/AVR timer functionality - no bit-twiddling needed!
* Extend timer ranges to 32 bits - about 268 seconds at 16 MHz with a precision of 1 clock cycle
* Precise pulse generator
* Input capture interrupts, similar to Arduino interrupts
* Tested on Uno, Mega2560. Could easily adapt for other AVR chips
* LGPL v3

## Examples

* code-timing - Count the number of clock cycles that a piece of code takes
* pulse-generator - Similar to the classic Blink example, but with precise, jitter-free timing
* blink-timing - Use an input capture unit to time the classic delay-based Blink example

## Usage

### TimerUtil

timerUtil.h provides a number of convenience functions so you don't have to mess with AVR registers.
Simply use digitalPinToTimer() to find the timer for a pin, or use timer names (TIMER0, TIMER1, etc.) to identify timers.

configureTimerClock(timer, clock) - set the clock speed of a timer, relative to the system clock.

configureTimerMode(timer, mode) - change the timer mode.

setInputCaptureNoiseCancellerEnabled(timer, enabled)
getInputCaptureNoiseCancellerEnabled(timer) - Enable or disable the noise canceller for input capture.
Adds a 4 clock cycle delay to the input capture.

hasInputCapture(timer)
clearInputCapture(timer)
setInputCaptureEdge(timer, edge)
getInputCapture(timer) - Allows you to poll for whether there is an input capture event instead of using
an interrupt.

getTimerConfig()
restoreTimerConfig(config) - save and restore the clock setting and mode of a timer. Useful when switching between PWM and Normal mode on the same timer/pin.

### ExtTimer

ExtTimer extends the range of Arduino's built-in timers.

Ex: 
ticksExtraRange_t ticks = ExtTimer1.get();

### PulseGen

PulseGen generates precise, jitter-free pulses on PWM pins.

