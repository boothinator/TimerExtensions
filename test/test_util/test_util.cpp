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

#include <timerUtil.h>

#define MAX_MESSAGE_LEN 255

char message[MAX_MESSAGE_LEN];

void setUp(void) {
}

void tearDown(void) {
}

void test_setTimerClock()
{
  setTimerClock(TIMER0, TimerClock::Clk);
  TEST_ASSERT_EQUAL(0b00000001, TCCR0B & 0b00000111);
  setTimerClock(TIMER0, TimerClock::ClkDiv8);
  TEST_ASSERT_EQUAL(0b00000010, TCCR0B & 0b00000111);
  setTimerClock(TIMER0, TimerClock::ClkDiv1024);
  TEST_ASSERT_EQUAL(0b00000101, TCCR0B & 0b00000111);

  setTimerClock(TIMER1, TimerClock::ClkDiv8);
  TEST_ASSERT_EQUAL(0b00000010, TCCR1B & 0b00000111);
  setTimerClock(TIMER1, TimerClock::ClkDiv1024);
  TEST_ASSERT_EQUAL(0b00000101, TCCR1B & 0b00000111);
  
  setTimerClock(TIMER2, TimerClock::Clk);
  TEST_ASSERT_EQUAL(0b00000001, TCCR2B & 0b00000111);
  setTimerClock(TIMER2, TimerClock::ClkDiv8);
  TEST_ASSERT_EQUAL(0b00000010, TCCR2B & 0b00000111);
  setTimerClock(TIMER2, TimerClock::ClkDiv1024);
  TEST_ASSERT_EQUAL(0b00000111, TCCR2B & 0b00000111);
}

void test_setTimerMode()
{
  setTimerMode(TIMER0, TimerMode::Normal);
  TEST_ASSERT_EQUAL(0b00000000, TCCR0A & 0b00000011);
  TEST_ASSERT_EQUAL(0b00000000, TCCR0B & 0b00001000);

  setTimerMode(TIMER1, TimerMode::Normal);
  TEST_ASSERT_EQUAL(0b00000000, TCCR1A & 0b00000011);
  TEST_ASSERT_EQUAL(0b00000000, TCCR1B & 0b00001000);

  setTimerMode(TIMER2, TimerMode::Normal);
  TEST_ASSERT_EQUAL(0b00000000, TCCR2A & 0b00000011);
  TEST_ASSERT_EQUAL(0b00000000, TCCR2B & 0b00001000);
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();    // IMPORTANT LINE!

  RUN_TEST(test_setTimerClock);
  RUN_TEST(test_setTimerMode);

  UNITY_END(); // stop unit testing
}

void loop() {
}

