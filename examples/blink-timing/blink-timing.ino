#include <TimerExtensions.h>

/*
 * Arduino (and AVR) chips often have input capture units that
 * can save the current time when an external event happens.
 * While this could be done in code, the input capture unit does
 * this in hardware with very little delay. Also, code might be
 * delayed by other interrupts, while the input capture unit cannot
 * be delayed by interrupts since it is a purely hardware device.
 * 
 * In this example, we'll time the classic Blink example using an
 * input capture pin. Conveniently, the pin doesn't care whether the
 * pin was turned on by your code or whether it was turned on by an
 * external source. We'll use the times recorded by the input capture
 * register to determine the amount of overhead in the Blink code.
 * 
 * The Blink example produces a signal that does not repeat precisely
 * every two seconds. Even though the two delay() calls each accurately
 * produces a 1 second delay, they don't account for the amount of time
 * it takes to turn the pin or or off, don't account for the interrupt
 * that makes millis() and micros() work, and certainly don't account
 * for anything that happens randomly in interrupts.
 * 
 * This example times how long the cycle actually is and calculates the
 * amount of overhead. Of course, reporting the overhead itself creates
 * additional overhead. Only the first reported overhead value represents
 * the amount of overhead of just the two calls to digitalWrite(). For every
 * cycle after that, the overhead value also inclues the overhead required
 * to send the overhead value over the serial port.
 * 
 * Also, note that we're counting individual clock cycles, but the times
 * we're comparing don't fit into the 16-bit input capture register.
 * To overcome this obstacle, we're extending the range of the register
 * by calling ExtTimer.extendTimeInPast(). This combines the captured time
 * with the number of times the timer has over overflowed. Doing so allows
 * us to compare times that are up to about 268 seconds apart to an accuracy
 * of 1/16th of a microsecond. This is very similar to how the Arduino
 * library can keep track of the elapsed milliseconds using only an 8-bit
 * timer.
 */

ticksExtraRange_t prevTicks = 0;

// Receive the captured time from the input capture register
void inputCaptureInterrupt(ticks16_t ticks)
{
  // Extend the range of the input capture value
  ticksExtraRange_t curTicks = ExtTimerPin8.extendTimeInPast(ticks);
  
  if (prevTicks != 0)
  {
    Serial.print("Overhead (CPU cycles): ");
    Serial.println(curTicks - prevTicks - 2 * F_CPU);
  }

  prevTicks = curTicks;
}

void setup() {
  Serial.begin(9600);

  uint8_t timer = inputCapturePinToTimer(8);
  
  // Configure the timer to run at the speed of the system clock
  ExtTimerPin8.configure(TimerClock::Clk);
  
  pinMode(8, OUTPUT);

  attachInputCaptureInterrupt(timer, inputCaptureInterrupt, RISING);
  ExtTimerPin8.resetOverflowCount();
}

void loop() {
  digitalWrite(8, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);             // wait for a second
  digitalWrite(8, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);             // wait for a second
}
