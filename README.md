# TimerExtensions

*Easy access to advanced Arduino/AVR timer functions, such as precise timing, pulse generation, and input event capture.*

Directly manipulating the timers/counters in Arduino and AVR is tedious and confusing. This library makes that simple by providing a number of convenience methods for accessing the timers. Easily attach interrupts to input capture events, extend the range of timers to 32 bits, or emit precisely timed pulses.

## Features

* Abstracts much of the Arduino/AVR timer functionality - no bit-twiddling needed!
* Extend timer ranges to 32 bits - about 268 seconds at 16 MHz with a precision of 1 clock cycle
* Precise pulse generator
* Input capture interrupts, similar to Arduino interrupts
* Tested on Uno, Mega2560. Could easily adapt to other AVR chips
* No dependency on Arduino framework.
* LGPL v3

## Examples

* code-timing - Count the number of clock cycles that a piece of code takes
* pulse-generator - Similar to the classic Blink example, but with precise, jitter-free timing
* blink-timing - Use an input capture unit to time the classic delay-based Blink example

## Usage

### TimerUtil

timerUtil.h

TimerUtil provides a number of convenience functions so you don't have to mess with AVR registers.
Simply use digitalPinToTimer() to find the timer for a pin, or use timer names (TIMER0, TIMER1, etc.) to identify timers.

`configureTimerClock(timer, clock)` - set the clock speed of a timer, relative to the system clock. Note
that a value of None means that the clock is stopped.

`configureTimerMode(timer, mode)` - change the timer mode.

`setInputCaptureNoiseCancellerEnabled(timer, enabled)
getInputCaptureNoiseCancellerEnabled(timer)` - Enable or disable the noise canceller for input capture.
Adds a 4 clock cycle delay to the input capture.

`hasInputCapture(timer)`
`clearInputCapture(timer)`
`setInputCaptureEdge(timer, edge)`
`getInputCapture(timer)` - Allows you to poll for whether there is an input capture event instead of using
an interrupt.

`clockCyclesPerTick(clock)`
`ticksToClockCycles(ticks, clock)`
`ticksToMilliseconds(ticks, clock)`
`ticksToMicroseconds(ticks, clock)`
`clockCyclesToTicks(clockCycles, clock)`
`millisecondsToTicks(milliseconds, clock)`
`microsecondsToTicks(microseconds, clock)` - conversion

`getTimerValue(timer)`
`setTimerValue(timer, ticks)` - get and set timer value. Most useful when the clock is stopped.

`getTimerConfig()`
`restoreTimerConfig(config)` - save and restore the clock setting and mode of a timer. Useful when switching between PWM and Normal mode on the same timer/pin.

### ExtTimer

extTimer.h

ExtTimer extends the range of Arduino's built-in timers.

Ex: 
`ticksExtraRange_t ticks = ExtTimer1.get();`

### Input Capture Interrupts

timerInterrupts.h

Similar to Arduino, except that you attach an interrup to a timer, and the function you provde needs to take a uint16_t argument that will hold the input capture value. Note that only 16-bit timers have input capture units.

`attachInputCaptureInterrupt(timer, func, edge)`
`detachInputCaptureInterrupt(uint8_t timer)`

### PulseGen

pulseGen.h

PulseGen generates precise, jitter-free pulses on PWM pins. Note that this only when a timer's clock is in Normal mode, and the pin is set for output.

Ex: 
`configureTimerClock(ExtTimerPin11.getTimer(), TimerClock::ClkDiv1024);`

`configureTimerMode(ExtTimerPin11.getTimer(), TimerMode::Normal);`

`pinMode(11, OUTPUT);`

`ticksExtraRange_t nowTicks = ExtTimerPin11.get();`

`PulseGenPin11.setStart(nowTicks + 50000);`

`PulseGenPin11.setEnd(nowTicks + 110000);`
