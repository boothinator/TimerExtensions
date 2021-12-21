// Test for PulseGen
// Copyright (C) 2021  Joshua Booth

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <Arduino.h>
#include <unity.h>

#include <pulseGen.h>

#define MAX_MESSAGE_LEN 255

char message[MAX_MESSAGE_LEN];

void setUp(void) {
}

void tearDown(void) {
// clean stuff up here
}

volatile bool compa;

void stateChangeCallback(PulseGen *pulse, void *data)
{
  compa = true;
  //Serial.print("State Change: ");
  //Serial.println((uint8_t) state);
}

void test_pulse()
{
  uint16_t ocr = 0;
  uint8_t *pocrl = (uint8_t *)&ocr;
  uint8_t *pocrh = pocrl + 1;
  uint8_t tccra = 0;
  uint8_t tccrb = 0b00000011;
  uint8_t tccrc = 0;
  uint8_t com1 = 7;
  uint8_t com0 = 6;
  uint8_t foc = 5;
  uint16_t tcnt = 0;
  uint8_t *ptcntl = (uint8_t *)&tcnt;
  uint8_t *ptcnth = ptcntl + 1;
  ExtTimer extTimer(ptcntl, ptcnth);

  PulseGen pulse(pocrl, pocrh, &tccra, &tccrb, &tccrc, com1, com0, foc, &extTimer);
  pulse.setStateChangeCallback(stateChangeCallback);

  unsigned long startTime, endTime;

  ticksExtraRange_t startTicks = 200;
  ticksExtraRange_t endTicks = 400;

  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::Idle, pulse.getState());

  startTime = micros();
  TEST_ASSERT_TRUE(pulse.setStart(startTicks));
  endTime = micros();
  
  //Serial.print("Microseconds for pulse.setStart(startTicks): ");
  //Serial.println(endTime - startTime);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::Idle, pulse.getState());

  startTime = micros();
  TEST_ASSERT_TRUE(pulse.setEnd(endTicks));
  endTime = micros();
  
  //Serial.print("Microseconds for pulse.setEnd(endTicks): ");
  //Serial.println(endTime - startTime);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledHigh, pulse.getState());

  // We are in range, so PulseGen should be able to schedule the high state immediately
  TEST_ASSERT_BIT_HIGH(com1, tccra);
  TEST_ASSERT_BIT_HIGH(com0, tccra);
  TEST_ASSERT_EQUAL_UINT16(startTicks, ocr);

  // Verify that we can't update the start because we're too close to the start time
  tcnt = startTicks - 1;

  bool startUpdated = pulse.setStart(startTicks + 1);
  TEST_ASSERT_FALSE(startUpdated);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledHigh, pulse.getState());

  // Verify that we can update the end because
  bool endUpdated = pulse.setEnd(endTicks);
  TEST_ASSERT_TRUE(endUpdated);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledHigh, pulse.getState());

  // Verify that we can't cancel because we're too close to the start time
  bool cancelled = pulse.cancel();
  TEST_ASSERT_FALSE(cancelled);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledHigh, pulse.getState());

  // Hit start time
  tcnt = startTicks;
  pulse.processCompareEvent();

  // PulseGen should have scheduled the low state
  TEST_ASSERT_BIT_HIGH(com1, tccra);
  TEST_ASSERT_BIT_LOW(com0, tccra);
  TEST_ASSERT_EQUAL_UINT16(endTicks, ocr);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledLow, pulse.getState());

  // Verify that we can schedule the next start state now that we've gone high and advanced the counter a bit
  tcnt += 1;

  startUpdated = pulse.setStart(startTicks + 1);
  TEST_ASSERT_TRUE(startUpdated);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledLow, pulse.getState());

  // Verify that we can't update the end because we're too close
  tcnt = endTicks - 1;

  endUpdated = pulse.setEnd(endTicks + 1);
  TEST_ASSERT_FALSE(endUpdated);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledLow, pulse.getState());

  // Verify that we can't cancel because we've already went high
  cancelled = pulse.cancel();
  TEST_ASSERT_FALSE(cancelled);
  TEST_ASSERT_BIT_HIGH(SREG_I, SREG);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::ScheduledLow, pulse.getState());

  // Hit end time
  tcnt = endTicks;
  pulse.processCompareEvent();

  // PulseGen should still be in previous low state
  TEST_ASSERT_BIT_HIGH(com1, tccra);
  TEST_ASSERT_BIT_LOW(com0, tccra);
  TEST_ASSERT_EQUAL_UINT16(endTicks, ocr);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::Idle, pulse.getState());

  // Hit the stale start time again after timer overflow
  extTimer.processOverflow();
  tcnt = startTicks;
  pulse.processCompareEvent();

  // PulseGen should still be in previous low state
  TEST_ASSERT_BIT_HIGH(com1, tccra);
  TEST_ASSERT_BIT_LOW(com0, tccra);
  TEST_ASSERT_EQUAL_UINT16(endTicks, ocr);
  TEST_ASSERT_EQUAL_UINT8(PulseGen::PulseState::Idle, pulse.getState());
}

volatile bool capt = false;
volatile uint16_t captVal;

ISR(TIMER5_CAPT_vect)
{
  captVal = ICR5;
  capt = true;
}

void test_pulse_real()
{
  pinMode(46, OUTPUT);
  pinMode(48, INPUT);
  digitalWrite(46, LOW);

  // Reset all counter settings
  // Normal counting mode, no output change on compare
  // Stop timer
  TCCR5A = 0;
  TCCR5B = 0;

  // Reset timer
  TCNT5 = 0;
  ExtTimer5.resetOverflowCount();

  PulseGen5A.setStateChangeCallback(stateChangeCallback);

  // Enable output compare interrupts
  TIMSK5 |= _BV(OCIE5A) | _BV(TOIE5) | _BV(ICIE5);

  // Use clk/1024 prescaler and start timer
  TCCR5B |= (_BV(CS52) | _BV(CS50));

  // Schedule for two overflows into the future
  ticksExtraRange_t start = (2UL << 17);
  ticksExtraRange_t end = start + (unsigned long)UINT16_MAX + 1000;

  TEST_ASSERT_EQUAL(LOW, digitalRead(48));
  TEST_ASSERT_EQUAL(PulseGen::PulseState::Idle, PulseGen5A.getState());
  
  TEST_ASSERT_TRUE(PulseGen5A.setStart(start));
  TEST_ASSERT_TRUE(PulseGen5A.setEnd(end));

  TEST_ASSERT_BIT_HIGH(COM5A1, TCCR5A);
  TEST_ASSERT_BIT_LOW(COM5A0, TCCR5A);

  compa = false;

  TEST_ASSERT_EQUAL(PulseGen::PulseState::WaitingToScheduleHigh, PulseGen5A.getState());
  TEST_ASSERT_BIT_HIGH(COM5A1, TCCR5A);
  TEST_ASSERT_BIT_LOW(COM5A0, TCCR5A);

  // Wait for COMPA interrupt
  while (!compa) {}
  compa = false;

  TEST_ASSERT_EQUAL(PulseGen::PulseState::ScheduledHigh, PulseGen5A.getState());
  TEST_ASSERT_BIT_HIGH(COM5A1, TCCR5A);
  TEST_ASSERT_BIT_HIGH(COM5A0, TCCR5A);

  // Capture rising edge
  TCCR5B |= _BV(ICES5);
  capt = false;

  // Wait for COMPA interrupt
  while (!compa) {}
  compa = false;

  // Wait for CAPT interrupt
  while (!capt) {}
  TEST_ASSERT_TRUE(capt);
  capt = false;

  TEST_ASSERT_UINT16_WITHIN(1, start, captVal);

  TEST_ASSERT_EQUAL(PulseGen::PulseState::WaitingToScheduleLow, PulseGen5A.getState());
  TEST_ASSERT_EQUAL(HIGH, digitalRead(48));
  TEST_ASSERT_BIT_HIGH(COM5A1, TCCR5A);
  TEST_ASSERT_BIT_HIGH(COM5A0, TCCR5A);

  // Capture falling edge
  TCCR5B &= ~_BV(ICES5);
  capt = false;

  // Wait for COMPA interrupt
  while(!compa) {}
  compa = false;

  TEST_ASSERT_EQUAL(HIGH, digitalRead(48));
  TEST_ASSERT_EQUAL(PulseGen::PulseState::ScheduledLow, PulseGen5A.getState());
  TEST_ASSERT_BIT_HIGH(COM5A1, TCCR5A);
  TEST_ASSERT_BIT_LOW(COM5A0, TCCR5A);

  // Wait for COMPA interrupt
  while (!compa) {}
  compa = false;

  // Wait for CAPT interrupt
  while (!capt) {}
  TEST_ASSERT_TRUE(capt);
  capt = false;

  TEST_ASSERT_UINT16_WITHIN(1, end, captVal);

  TEST_ASSERT_EQUAL(LOW, digitalRead(48));
  TEST_ASSERT_EQUAL(PulseGen::PulseState::Idle, PulseGen5A.getState());
  TEST_ASSERT_BIT_HIGH(COM5A1, TCCR5A);
  TEST_ASSERT_BIT_LOW(COM5A0, TCCR5A);
}

bool icpConnected()
{
  pinMode(46, INPUT);
  pinMode(48, OUTPUT);

  digitalWrite(48, LOW);
  if (LOW != digitalRead(46))
  {
    return false;
  }

  digitalWrite(48, HIGH);
  if (HIGH != digitalRead(46))
  {
    return false;
  }

  return true;
}

void test_icp_connected()
{
  pinMode(46, INPUT);
  pinMode(48, OUTPUT);

  digitalWrite(48, LOW);
  TEST_ASSERT_EQUAL_MESSAGE(LOW, digitalRead(46), "Please connect Pin 46 and 48");

  digitalWrite(48, HIGH);
  TEST_ASSERT_EQUAL_MESSAGE(HIGH, digitalRead(46), "Please connect Pin 46 and 48");
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();    // IMPORTANT LINE!



  for (int i = 0; i < 10 && false == icpConnected(); i++)
  {
    Serial.println("Please connect Pin 46 and 48");
    delay(2000);
  }

  RUN_TEST(test_icp_connected);

  RUN_TEST(test_pulse);
  RUN_TEST(test_pulse_real);

  // TODO: test when extended timer overflows

  UNITY_END(); // stop unit testing
}

void loop() {
}

