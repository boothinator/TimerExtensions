# ArduinoTimerExtensions

Easy access to advanced Arduino timer functions, such as precise timing, pulse generation, and input event capture.

## Examples

See the Examples folder for example code.

* code-timing.ino - Count the number of clock cycles that a piece of code takes
* pulse-generator - Similar to the classic Blink example, but with precise, jitter-free timing

## Library

### TimerUtil

timerUtil.h provides a number of convenience functions so you don't have to mess with AVR registers.

Use configureTimerClock() to set the clock speed of a timer, relative to the system clock.

Use configureTimerMode() to change the timer mode.

Use getTimerConfig() and restoreTimerConfig() to save and restore the clock and mode of a timer.

### ExtTimer

ExtTimer extends the range of Arduino's built-in timers.

Ex: 
ticksExtraRange_t ticks = ExtTimer1.get();

### PulseGen

PulseGen generates precise, jitter-free pulses on PWM pins.

