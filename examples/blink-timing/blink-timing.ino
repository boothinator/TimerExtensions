#include <timerUtil.h>
#include <timerInterrupts.h>

ticks16_t prevTicks = 0;

void inputCaptureInterrupt(ticks16_t ticks)
{
  Serial.println(ticks - prevTicks);

  prevTicks = ticks;
}

void setup() {
  Serial.begin(115200);

  // GetTimer(pin 8)
  
  // Configure the timer to run at the speed of the system clock
  configureTimerClock(Timer::Timer1, TimerClock::ClkDiv1024);

  // Put timer in Normal timing mode
  configureTimerMode(Timer::Timer1, TimerMode::Normal);
  
  pinMode(8, OUTPUT);

  attachInputCaptureInterrupt(Timer::Timer1, inputCaptureInterrupt, Edge::Rising);
}

void loop() {
  digitalWrite(8, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);             // wait for a second
  digitalWrite(8, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);             // wait for a second
}
