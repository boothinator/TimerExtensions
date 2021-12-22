#include <timerUtil.h>
#include <timerInterrupts.h>
#include <extTimer.h>

ticksExtraRange_t prevTicks = 0;

void inputCaptureInterrupt(ticks16_t ticks)
{
  ticksExtraRange_t curTicks = ExtTimerPin8.extendTimeInPast(ticks);
  
  if (prevTicks != 0)
  {
    Serial.print("Overhead (cycles): ");
    Serial.println(curTicks - prevTicks - 2 * F_CPU);
  }

  prevTicks = curTicks;
}

void setup() {
  Serial.begin(115200);

  uint8_t timer = TIMER1;
  
  // Configure the timer to run at the speed of the system clock
  configureTimerClock(timer, TimerClock::Clk);

  // Put timer in Normal timing mode
  configureTimerMode(timer, TimerMode::Normal);
  
  pinMode(8, OUTPUT);

  attachInputCaptureInterrupt(timer, inputCaptureInterrupt, RISING);
}

void loop() {
  digitalWrite(8, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);             // wait for a second
  digitalWrite(8, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);             // wait for a second
}
