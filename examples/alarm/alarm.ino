#include <TimerExtensions.h>

/*
 * It's pretty easy to set an alarm, even one that's quite far in the future.
 * At best, Arduino/AVR timers can set an alarm a little over 4 seconds into the
 * future. We can use a TimerAction to set an alarm much further. Try it out!
 */

void alarm(TimerAction *, void *) {
  Serial.println("Hello world!");
}

void setup() {
  Serial.begin(9600);

  TimerAction1A.configure(TimerClock::ClkDiv8);

  ticksExtraRange_t nowTicks = TimerAction1A.getNow();

  ticksExtraRange_t alarmTicks = TimerAction1A.millisecondsToTicks(8000);

  TimerAction1A.schedule(alarmTicks, alarm);

  Serial.println("Alarm scheduled");
}
  
void loop() {
}