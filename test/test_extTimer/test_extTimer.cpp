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
  ExtTimer0.configure(TimerClock::Clk);
  ExtTimer1.configure(TimerClock::Clk);
  ExtTimer2.configure(TimerClock::Clk);
}

void tearDown(void) {
}

void test_configure()
{
  TEST_ASSERT_BIT_HIGH(TOIE0, TIMSK0);
  TEST_ASSERT_BIT_HIGH(TOIE1, TIMSK1);
  TEST_ASSERT_BIT_HIGH(TOIE2, TIMSK2);
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

void test_uniformIncreasing(ExtTimer &extTimer)
{
  extTimer.resetOverflowCount();
  ticksExtraRange_t prevTicks = extTimer.get();
  ticksExtraRange_t prevOvfTicks = extTimer.getOverflowTicks();

  for (unsigned long i = 0; i < 10000 && prevOvfTicks < (1ul << 30); i++)
  {
    ticksExtraRange_t curTicks = extTimer.get();
    ticksExtraRange_t curOvfTicks = extTimer.getOverflowTicks();

    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(prevTicks, curTicks);

    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(prevOvfTicks, curOvfTicks);
    
    prevTicks = curTicks;
    prevOvfTicks = curOvfTicks;
  }
}

void test_set8Bit()
{
  ticksExtraRange_t expected = 0xabcdef01;
  ticks8_t expected8 = static_cast<ticks8_t>(expected);
  ticksExtraRange_t expectedOverflowCount = (expected & 0xFFFFFF00) >> 8;

  setTimerClock(TIMER0, TimerClock::None);

  // Set TOV flag
  TIFR0 = (1 << TOV0);

  ExtTimer0.set(expected);

  uint8_t tcnt = TCNT0;

  TEST_ASSERT_EQUAL_UINT8(expected8, tcnt);
  TEST_ASSERT_EQUAL_UINT32(expected, ExtTimer0.get());
  TEST_ASSERT_EQUAL_UINT32(ExtTimer0.getOverflowCount(), expectedOverflowCount);

  bool tov = (TIFR0 & (1 << TOV0)) == (1 << TOV0);

  TEST_ASSERT_FALSE(tov);
}

void test_uniformIncreasing0()
{
  test_uniformIncreasing(ExtTimer0);
}

void test_uniformIncreasing1()
{
  test_uniformIncreasing(ExtTimer1);
}

void test_uniformIncreasing2()
{
  test_uniformIncreasing(ExtTimer2);
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();    // IMPORTANT LINE!

  RUN_TEST(test_configure);
  RUN_TEST(test_set);
  RUN_TEST(test_uniformIncreasing1);
  RUN_TEST(test_set8Bit);
  RUN_TEST(test_uniformIncreasing0);
  RUN_TEST(test_uniformIncreasing2);

  UNITY_END(); // stop unit testing
}

void loop() {
}

