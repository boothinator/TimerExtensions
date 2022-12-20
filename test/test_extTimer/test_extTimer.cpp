// Test for TimerAction
// Copyright (C) 2022  Joshua Booth

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

#include <extTimer.h>
#include <timerUtil.h>

#define MAX_MESSAGE_LEN 255

char message[MAX_MESSAGE_LEN];

void setUp(void) {
  setTimerMode(TIMER1, TimerMode::Normal);
  setTimerClock(TIMER1, TimerClock::Clk);
}

void tearDown(void) {
}

void test_set()
{
  ticksExtraRange_t expected = 0xabcdef01;
  ticks16_t expected16 = static_cast<ticks16_t>(expected);
  ticksExtraRange_t expectedOverflowCount = (expected & 0xFFFF0000) >> 16;

  setTimerClock(TIMER1, TimerClock::None);

  // Set TOV flag
  TIFR1 = (1 << TOV1);

  ExtTimer1.set(expected);

  uint16_t tcnt = TCNT1;

  TEST_ASSERT_EQUAL_UINT16(expected16, tcnt);
  TEST_ASSERT_EQUAL(expected, ExtTimer1.get());
  TEST_ASSERT_EQUAL(ExtTimer1.getOverflowCount(), expectedOverflowCount);

  bool tov = (TIFR1 & (1 << TOV1)) == (1 << TOV1);

  TEST_ASSERT_FALSE(tov);
}

void test_uniformIncreasing()
{
  ticksExtraRange_t prevTicks = ExtTimer1.get();

  for (unsigned long i = 0; i < 1000000; i++)
  {
    ticksExtraRange_t curTicks = ExtTimer1.get();
    TEST_ASSERT_GREATER_THAN(prevTicks, curTicks);
    prevTicks = curTicks;
  }
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  //delay(2000);
  delay(100);

  UNITY_BEGIN();    // IMPORTANT LINE!

  RUN_TEST(test_set);
  RUN_TEST(test_uniformIncreasing);

  UNITY_END(); // stop unit testing
}

void loop() {
}

