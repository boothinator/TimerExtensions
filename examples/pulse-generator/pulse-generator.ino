#include <pulseGen.h>
#include <extTimer.h>
#include <timerUtil.h>

/* 
 * Arduino (and AVR) timers are simple counters that are synchronized with
 * some multiple of the system clock. One important purpose is to control
 * a pin with extreme accuracy. This is commonly done using analogWrite()
 * to create a PWM output, but this can also be used to switch pins on and
 * off at arbitrary times with great accuracy and precision.
 * 
 * In this example, we'll do just that: turn a pin on and off with great
 * accuracy. We'll do the necessary setup to control pin 11, and then schedule
 * a pulse. Simply connect an LED and current-limiting resistor between pin 11
 * and ground to see the result.
 * 
 * Note that the on and off states are accurate to within 64 microseconds, and
 * that turning the light on and off cannot be delayed by an interrupt. Well,
 * that's only mostly true. The library utilizes interrupts to schedule the
 * on and off transitions at the correct time. However, the state change cannot
 * be delayed or affected by any interrupts once scheduled. This uses the same
 * mechanism as analogWrite() to control when the pin turns on and off. Namely,
 * the Output Compare Register (OCR) is set to the correct time for each event.
 * 
 * Also note that the times are much bigger than the 256 values allowed by
 * analogWrite(), and bigger than the 65536 values allowed by the OCRs. This
 * library counts timer overflows, extending the range of values to a 32-bit
 * number. This allows you to schedule events well into the future at great
 * accuracy.
 */

void setup() {
  Serial.begin(115200);
  // Configure the timer to run at the speed of the system clock, divided by 1024
  // That's 64 microseconds per tick
  configureTimerClock(ExtTimerPin11.getTimer(), TimerClock::ClkDiv1024);

  // Put timer in Normal timing mode
  configureTimerMode(ExtTimerPin11.getTimer(), TimerMode::Normal);

  // Configure pin for output
  pinMode(11, OUTPUT);
}

void loop() {
  // Only reschedule if it's idle
  if (PulseGenPin11.getState() == PulseGen::Idle)
  {
    // Get current ticks for timing
    ticksExtraRange_t nowTicks = ExtTimerPin11.get();
  
    // Schedule pulse start for 50,000 ticks in the future. That's 3.2 seconds.
    PulseGenPin11.setStart(nowTicks + 50000);
    
    // Schedule pulse end for 110,000 ticks in the future. That's 6.6 seconds, 
    // for a pulse length of 3.4 seconds, accurate to within 1/16th of a microsecond
    PulseGenPin11.setEnd(nowTicks + 110000);
  }
}
