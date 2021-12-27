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

TimerUtil provides a number of convenience functions so you don't have to mess with AVR registers. Simply use digitalPinToTimer() to find the timer for a pin, or use timer names (TIMER0, TIMER1, etc.) to identify timers.

#### Clock and Mode

`configureTimerClock(timer, clock)` - set the clock speed of a timer, relative to the CPU clock. Note that a value of None means that the clock is stopped.

`configureTimerMode(timer, mode, resolution)` - change the timer mode.

`getTimerValue(timer)`  
`setTimerValue(timer, ticks)` - get and set timer value. Most useful when the clock is stopped, and in Normal, CTC, or Fast PWM modes.

#### Time Conversions

`clockCyclesPerTick(clock)`  
`ticksToClockCycles(ticks, clock)`  
`ticksToMilliseconds(ticks, clock)`  
`ticksToMicroseconds(ticks, clock)`  
`clockCyclesToTicks(clockCycles, clock)`  
`millisecondsToTicks(milliseconds, clock)`  
`microsecondsToTicks(microseconds, clock)`  

#### Input Capture

Input capture pins allow you to measure the precise time of an external event. The value of of the timer is stored when the correct edge is detected (RISING or FALLING), and this is done independently of the CPU or interrupts. This can be used to determine the exact time of the event.  Best used with normal timing mode.

`hasInputCapture(timer)`  
`clearInputCapture(timer)`  
`setInputCaptureEdge(timer, edge)`  
`getInputCapture(timer, clear = true)` - Allow you to poll for whether there is an input capture event.

`setInputCaptureNoiseCancellerEnabled(timer, enabled)`  
`getInputCaptureNoiseCancellerEnabled(timer)` - Enable or disable the noise canceller for input capture. Adds a delay of 4 CPU clock cycles to the input capture.

Ex:

```C++
configureTimerMode(TIMER1, TimerMode::Normal);
while (!hasInputCapture(TIMER1))
{
  uint16_t ticks = getInputCapture(TIMER1);
}
```

The UNO has one input capture pin on pin 8, which is linked to TIMER1.

The Mega two input capture pins that you can directly access: pin 48 (TIMER5) and 49 (TIMER4). You can indirectly use pin 5 (TIMER3) as an input capture pin, but you need to configure the Analog Comparator to do so. Also note that the pin 5 input capture edges will be inverted. The Mega does not expose the TIMER1 input capture pin.

Analog Comparator Example:

```C++
configureTimerMode(TIMER3, TimerMode::Normal);
ACSR |= _BV(ACBG) | _BV(ACIC);
while (!hasInputCapture(TIMER3))
{
  uint16_t ticks = getInputCapture(TIMER3);
}
```

#### Timer Reset

Sometimes you need to reset several timers to the same value so they move in time with each other. These functions allow you to stop all of the timers, configure them, and then start them all at the same time.

Timers generally feed off of a "prescaler" that takes the CPU clock and divides it by a certain amount. These functions work by continually resetting the prescaler that feeds the synchronous timers (most of them) and the prescaler that feeds the asynchronous timer (TIMER2). You can then configure the timers, then start them all at once.

`resetSynchronousPrescaler()`  
`resetAsynchronousPrescaler()`  
`setTimerSynchronizationModeEnabled(bool enabled)`  
`stopAllTimersAndSynchronize()`  
`startAllTimers()` - Reset and synchronized timers

```C++
stopAllTimersAndSynchronize();
configureTimerMode(TIMER1, TimerMode::Normal);
configureTimerMode(TIMER3, TimerMode::Normal);
setTimerValue(TIMER1, 0);
setTimerValue(TIMER3, 0);
startAllTimers();
// TIMER1 and TIMER3 will now always have the same value
```

#### Timer Config Save and Restore

`getTimerConfig()`  
`restoreTimerConfig(config)` - save and restore the clock setting and mode of a timer. Useful when switching between PWM and Normal mode on the same timer/pin.

### ExtTimer

extTimer.h

ExtTimer extends the range of Arduino's built-in timers. Use with the Normal timer mode.

Ex: 

```C++
configureTimerMode(ExtTimer1.getTimer(), TimerMode::Normal);
ticksExtraRange_t ticks = ExtTimer1.get();
```

### Input Capture Interrupts

timerInterrupts.h

The interrupt interface is similar to the interface in Arduino, except that you attach an interrupt to a timer, and the function you provde needs to take a uint16_t argument that will hold the input capture value.

`attachInputCaptureInterrupt(timer, func, edge)`  
`detachInputCaptureInterrupt(uint8_t timer)`

### PulseGen

pulseGen.h

PulseGen generates precise, jitter-free pulses on PWM pins. Note that this only when a timer's clock is in Normal mode, and the pin is set for output.

Note that PulseGen does not automatically set the pin mode.

Ex: 
```C++
configureTimerClock(ExtTimerPin11.getTimer(), TimerClock::ClkDiv1024);
configureTimerMode(ExtTimerPin11.getTimer(), TimerMode::Normal);
pinMode(11, OUTPUT);
ticksExtraRange_t nowTicks = ExtTimerPin11.get();
PulseGenPin11.setStart(nowTicks + 50000);
PulseGenPin11.setEnd(nowTicks + 110000);
```
