#include <timerUtil.h>

/*
 * Arduino (and AVR) timers are simple counters that are synchronized with
 * some multiple of the system clock. This makes them useful for counting
 * exactly how many clock cycles code runs.
 * 
 * In this example, we'll repurpose the 16-bit Timer 1 to time how long it takes
 * to send "Hello World" over the serial line. Normally, Timer 1 (along with
 * other timers) is used to time PWM signals that are generated by analogWrite().
 * However, we'll change the timer counting mode to "Normal" and change the
 * timer to count up once per clock pulse. This will allow us to time events.
 * 
 * To actually time an event, we save the state of the TCNT1 register just
 * before the event we want to time, and we then save it again just after the
 * event. Taking the difference will tell us exactly how many clock cycles
 * elapsed. (Well, plus one extra cycle, for esoteric reasons.)
 * 
 * Using Timer 1 in this way interferes with all of the PWM signals for pins
 * associated with Timer 1. For the Arduino Uno, this interferes with
 * analogWrite() on pins 9 and 10. To account for this, you can use
 * getTimerConfig() to save the configuration of Timer 1 before using it
 * in this way, and then use restoreTimerConfig() to restore the original
 * configuration. Doing these steps will allow you to use PWM again after
 * you are finished using the timer for other purposes.
 */
void setup() {
  Serial.begin(9600);

  // Configure Timer1 to run at the speed of the system clock
  setTimerClock(TIMER1, TimerClock::Clk);

  // Put timer in Normal timing mode
  setTimerMode(TIMER1, TimerMode::Normal);

  unsigned int startTicks = TCNT1;
  Serial.println("Hello World");
  unsigned int endTicks = TCNT1;

  Serial.print("Printing \"Hello World\" took ");
  Serial.print(endTicks - startTicks);
  Serial.println(" clock cycles");
}

void loop() {
}
