// Test for PulseGen
// Copyright (C) 2021  Joshua Booth

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser Public License for more details.

// You should have received a copy of the GNU Lesser Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <Arduino.h>
#include <unity.h>

#include <pulseGen.h>
#include <timerInterrupts.h>
#include <timerUtil.h>

#define MAX_MESSAGE_LEN 255

char message[MAX_MESSAGE_LEN];

void setUp(void) {
  pinMode(11, OUTPUT);

  setTimerClock(TIMER1, TimerClock::Clk);
  setTimerMode(TIMER1, TimerMode::Normal);
}

void tearDown(void) {
  setOutputCompareAction(TIMER1A, CompareAction::Nothing);
}

int digitalReadPWM(uint8_t pin)
{
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);

	if (*portInputRegister(port) & bit) return HIGH;
	return LOW;
}

void test_pulse()
{
  PulseGen pulseGen(TimerAction1A);

  ticksExtraRange_t start = ExtTimer1.get() + 2000;
  ticksExtraRange_t end = start + 2000;

  TEST_ASSERT_TRUE(pulseGen.schedule(start, end));

  TEST_ASSERT_EQUAL(PulseGen::ScheduledStart, pulseGen.getState());

  while(pulseGen.getState() == PulseGen::ScheduledStart && ExtTimer1.get() < start) {}

  TEST_ASSERT_EQUAL(HIGH, digitalReadPWM(11));

  TEST_ASSERT_EQUAL(PulseGen::ScheduledEnd, pulseGen.getState());
  
  while(pulseGen.getState() == PulseGen::ScheduledEnd && ExtTimer1.get() < end) {}

  TEST_ASSERT_EQUAL(PulseGen::Idle, pulseGen.getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(11));


}

void test_pulse_missStart()
{
  PulseGen pulseGen(TimerAction1A);

  ticksExtraRange_t start = ExtTimer1.get() + 100;
  ticksExtraRange_t end = start + 2000;

  TEST_ASSERT_FALSE(pulseGen.schedule(start, end));

  TEST_ASSERT_EQUAL(PulseGen::MissedStart, pulseGen.getState());
}

void test_pulse_missEnd()
{
  PulseGen pulseGen(TimerAction1A);

  ticksExtraRange_t start = ExtTimer1.get() + 2000;
  ticksExtraRange_t end = start + 10;

  TEST_ASSERT_TRUE(pulseGen.schedule(start, end));

  TEST_ASSERT_EQUAL(PulseGen::ScheduledStart, pulseGen.getState());

  while(pulseGen.getState() == PulseGen::ScheduledStart && ExtTimer1.get() < start) {}

  TEST_ASSERT_EQUAL(HIGH, digitalReadPWM(11));

  TEST_ASSERT_EQUAL(PulseGen::MissedEnd, pulseGen.getState());
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  //delay(2000);
  delay(100);

  UNITY_BEGIN();    // IMPORTANT LINE!

  RUN_TEST(test_pulse);
  RUN_TEST(test_pulse_missStart);
  RUN_TEST(test_pulse_missEnd);

  UNITY_END(); // stop unit testing
}

void loop() {
}

