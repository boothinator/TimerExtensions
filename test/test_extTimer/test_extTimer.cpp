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

bool overflowCallbackCalled = false;

void overflowCallback()
{
  overflowCallbackCalled = true;
}

void setUp(void) {
  overflowCallbackCalled = false;
  ExtTimer0.configure(TimerClock::Clk);
  ExtTimer1.configure(TimerClock::Clk);
  ExtTimer2.configure(TimerClock::Clk);

  ExtTimer1.setOverflowCallback(overflowCallback);
  ExtTimer2.setOverflowCallback(overflowCallback);
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

    if (&extTimer != &ExtTimer0 && curOvfTicks > 0)
    {
      TEST_ASSERT_TRUE(overflowCallbackCalled);
    }
    
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

void test_extTimer16()
{
  uint8_t tcntl = 0;
  uint8_t tcnth = 0;
  uint8_t timsk = 0;
  uint8_t toie = 0;
  uint8_t tifr = 0;
  uint8_t tov = 0;
  uint8_t timer = 0;

  ExtTimer extTimer(&tcntl, &tcnth, &timsk, toie, &tifr, tov, timer);

  TEST_ASSERT_BIT_HIGH(toie, timsk);

  // Initial state: all zeroes
  TEST_ASSERT_EQUAL(0, extTimer.get());
  TEST_ASSERT_EQUAL(0, extTimer.getOverflowCount());
  TEST_ASSERT_EQUAL(0, extTimer.getOverflowTicks());

  // Test overflow
  extTimer.processOverflow();

  TEST_ASSERT_EQUAL_HEX32(0x00010000, extTimer.get());
  TEST_ASSERT_EQUAL(1, extTimer.getOverflowCount());
  TEST_ASSERT_EQUAL_HEX32(0x00010000, extTimer.getOverflowTicks());
  TEST_ASSERT_EQUAL_HEX32(0x000100FF, extTimer.extend(0x00FF));
  TEST_ASSERT_EQUAL_HEX32(0x000000FF, extTimer.extendTimeInPast(0x00FF));

  // Test extend
  tcntl = 0x00;
  tcnth = 0x01;
  tifr = 0;

  TEST_ASSERT_EQUAL_HEX32(0x00010100, extTimer.get());
  TEST_ASSERT_EQUAL_HEX32(0x00010000, extTimer.getOverflowTicks());
  TEST_ASSERT_EQUAL_HEX32(0x000200FF, extTimer.extend(0x00FF));
  TEST_ASSERT_EQUAL_HEX32(0x000100FF, extTimer.extendTimeInPast(0x00FF));

  // Test overflow again
  extTimer.processOverflow();
  
  tcntl = 0x00;
  tcnth = 0x00;
  tifr = 0;

  TEST_ASSERT_EQUAL_HEX32(0x00020000, extTimer.get());
  TEST_ASSERT_EQUAL_HEX32(0x00020000, extTimer.getOverflowTicks());
  TEST_ASSERT_EQUAL_HEX32(0x000200FF, extTimer.extend(0x00FF));
  TEST_ASSERT_EQUAL_HEX32(0x000100FF, extTimer.extendTimeInPast(0x00FF));
  
  // Test unprocessed overflow flag
  tcntl = 0x00;
  tcnth = 0x00;
  tifr |= 1 << tov;

  TEST_ASSERT_EQUAL_HEX32(0x00030000, extTimer.get());
  TEST_ASSERT_EQUAL_HEX32(0x00030000, extTimer.getOverflowTicks());
  TEST_ASSERT_EQUAL_HEX32(0x000300FF, extTimer.extend(0x00FF));
  TEST_ASSERT_EQUAL_HEX32(0x000200FF, extTimer.extendTimeInPast(0x00FF));
  
  // Process overflow
  tcntl = 0x00;
  tcnth = 0x00;
  tifr = 0;
  extTimer.processOverflow();

  TEST_ASSERT_EQUAL_HEX32(0x00030000, extTimer.get());
  TEST_ASSERT_EQUAL_HEX32(0x00030000, extTimer.getOverflowTicks());
  TEST_ASSERT_EQUAL_HEX32(0x000300FF, extTimer.extend(0x00FF));
  TEST_ASSERT_EQUAL_HEX32(0x000200FF, extTimer.extendTimeInPast(0x00FF));
  
  // Test when extending a time that equals now
  tcntl = 0xFF;
  tcnth = 0x00;
  tifr = 0;

  TEST_ASSERT_EQUAL_HEX32(0x000300FF, extTimer.get());
  TEST_ASSERT_EQUAL_HEX32(0x00030000, extTimer.getOverflowTicks());
  TEST_ASSERT_EQUAL_HEX32(0x000300FF, extTimer.extend(0x00FF));
  TEST_ASSERT_EQUAL_HEX32(0x000200FF, extTimer.extendTimeInPast(0x00FF));
  
  // Test when extending a time that is smaller than now
  tcntl = 0x00;
  tcnth = 0x01;
  tifr = 0;

  TEST_ASSERT_EQUAL_HEX32(0x00030100, extTimer.get());
  TEST_ASSERT_EQUAL_HEX32(0x00030000, extTimer.getOverflowTicks());
  TEST_ASSERT_EQUAL_HEX32(0x000400FF, extTimer.extend(0x00FF));
  TEST_ASSERT_EQUAL_HEX32(0x000300FF, extTimer.extendTimeInPast(0x00FF));
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
  RUN_TEST(test_extTimer16);

  UNITY_END(); // stop unit testing
}

void loop() {
}

