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

#include <timerAction.h>
#include <timerInterrupts.h>
#include <timerUtil.h>

#define MAX_MESSAGE_LEN 255

char message[MAX_MESSAGE_LEN];

void setUp(void) {
}

void tearDown(void) {
  setOutputCompareAction(TIMER1A, CompareAction::Nothing);
}

void scheduleAndTest(ticksExtraRange_t actionTicks, CompareAction action)
{
  int pinStartState = digitalRead(11);

  TimerAction1A.schedule(actionTicks, action);

  ticksExtraRange_t startTicks = ExtTimer1.get();

  bool timedOut = false;

  while ((
      TimerAction1A.getState() == TimerAction::Scheduled
      || TimerAction1A.getState() == TimerAction::WaitingToSchedule
    )
    && !timedOut)
  {
    timedOut = !(ExtTimer1.get() - startTicks < actionTicks - startTicks + 1000);
  }

  snprintf(message, MAX_MESSAGE_LEN, "scheduleAndTest() ActionTicks: %lx, CurTicks: %lx, Action: %u, State: %u",
    actionTicks, ExtTimer1.get(), action, TimerAction1A.getState());

  TEST_ASSERT_FALSE_MESSAGE(timedOut, message);

  TEST_ASSERT_EQUAL_MESSAGE(TimerAction::Idle, TimerAction1A.getState(), message);

  if (CompareAction::Set == action)
  {
    TEST_ASSERT_EQUAL_MESSAGE(HIGH, digitalRead(11), message);
  }
  else if (CompareAction::Clear == action)
  {
    TEST_ASSERT_EQUAL_MESSAGE(LOW, digitalRead(11), message);
  }
  else if (CompareAction::Toggle == action)
  {
    TEST_ASSERT_NOT_EQUAL_MESSAGE(pinStartState, digitalRead(11), message);
  }
}

void test_basic()
{
  scheduleAndTest(ExtTimer1.get() + 10000ul, CompareAction::Set);
  scheduleAndTest(ExtTimer1.get() + 10000ul, CompareAction::Clear);
  scheduleAndTest(ExtTimer1.get() + 10000ul, CompareAction::Toggle);
  scheduleAndTest(ExtTimer1.get() + 1000000ul, CompareAction::Set);
  scheduleAndTest(ExtTimer1.get() + 1000000ul, CompareAction::Clear);
  scheduleAndTest(ExtTimer1.get() + 1000000ul, CompareAction::Toggle);
}

void test_timerOverflow()
{
  const ticksExtraRange_t actionTicks = 100ul;
  ExtTimer1.set(actionTicks - 1000ul);
  scheduleAndTest(actionTicks, CompareAction::Set);
}

void test_shortmiss()
{
  ticksExtraRange_t startTicks = ExtTimer1.get();

  const ticksExtraRange_t actionTicks = startTicks + 200ul;

  TimerAction1A.schedule(actionTicks, CompareAction::Set);

  TEST_ASSERT_EQUAL(TimerAction::Idle, TimerAction1A.getState());

	uint8_t bit = digitalPinToBitMask(11);
	uint8_t port = digitalPinToPort(11);

  TEST_ASSERT_TRUE(*portInputRegister(port) & bit);
}

void test_longmiss()
{
  // Force a miss by scheduling something further into the future than the system
  // register allows and turning off interrupts until after the action time
  noInterrupts();

  ticksExtraRange_t startTicks = ExtTimer1.get();

  const ticksExtraRange_t actionTicks = startTicks + ExtTimer1.getMaxSysTicks() + 1000ul;
  TimerAction1A.schedule(actionTicks, CompareAction::Set);

  while (ExtTimer1.get() - startTicks < actionTicks - startTicks) {}
  
  // Now that the action time has passed, enable interrupts and let it process
  interrupts();
  while (ExtTimer1.get() - startTicks < actionTicks + ExtTimer1.getMaxSysTicks() + 1000ul - startTicks) {}

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, TimerAction1A.getState());
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  //delay(2000);

  pinMode(11, OUTPUT);

  configureTimerClock(TIMER1, TimerClock::Clk);
  configureTimerMode(TIMER1, TimerMode::Normal);

  UNITY_BEGIN();    // IMPORTANT LINE!

  //RUN_TEST(test_basic);
  //RUN_TEST(test_timerOverflow);
  //RUN_TEST(test_longmiss);
  RUN_TEST(test_shortmiss);

  UNITY_END(); // stop unit testing
}

void loop() {
}

