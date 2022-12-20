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


void scheduleAndTest(ticksExtraRange_t actionTicks, CompareAction action)
{

  int pinStartState = digitalReadPWM(11);

  ticksExtraRange_t startTicks = ExtTimer1.get();

  TEST_ASSERT_TRUE(TimerAction1A.schedule(actionTicks, action));

  bool timedOut = false;

  while ((
      TimerAction1A.getState() == TimerAction::Scheduled
      || TimerAction1A.getState() == TimerAction::WaitingToSchedule
    )
    && !timedOut)
  {
    timedOut = ExtTimer1.get() - startTicks > actionTicks - startTicks + 1000;
  }

  snprintf(message, MAX_MESSAGE_LEN, "scheduleAndTest() ActionTicks: %lx, CurTicks: %lx, OriginTicks: %lx, Action: %u, State: %u",
    actionTicks, ExtTimer1.get(), TimerAction1A.getOriginTicks(), action, TimerAction1A.getState());

  TEST_ASSERT_FALSE_MESSAGE(timedOut, message);

  TEST_ASSERT_EQUAL_MESSAGE(TimerAction::Idle, TimerAction1A.getState(), message);

  if (CompareAction::Set == action)
  {
    TEST_ASSERT_EQUAL_MESSAGE(HIGH, digitalReadPWM(11), message);
  }
  else if (CompareAction::Clear == action)
  {
    TEST_ASSERT_EQUAL_MESSAGE(LOW, digitalReadPWM(11), message);
  }
  else if (CompareAction::Toggle == action)
  {
    TEST_ASSERT_NOT_EQUAL_MESSAGE(pinStartState, digitalReadPWM(11), message);
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

void test_timerOverflowOrigin()
{
  ticksExtraRange_t originTicks = UINT32_MAX - 10000ul;
  ExtTimer1.set(originTicks);

  ticksExtraRange_t actionTicks = 100ul;

  TEST_ASSERT_TRUE(TimerAction1A.schedule(actionTicks, CompareAction::Set, originTicks));

  while (ExtTimer1.get() - originTicks < actionTicks - originTicks) {}

  TEST_ASSERT_EQUAL(TimerAction::Idle, TimerAction1A.getState());

  TEST_ASSERT_EQUAL(HIGH, digitalReadPWM(11));
}

void test_supershortmiss()
{
  // Schedule something without enough lead time
  ticksExtraRange_t startTicks = ExtTimer1.get();

  ticksExtraRange_t actionTicks = startTicks + 100ul;

  TEST_ASSERT_FALSE(TimerAction1A.schedule(actionTicks, CompareAction::Set, startTicks));

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, TimerAction1A.getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(11));
}

void test_shortmiss()
{
  // Schedule something without enough lead time
  ticksExtraRange_t startTicks = ExtTimer1.get();

  ticksExtraRange_t actionTicks = startTicks + 200ul;

  TEST_ASSERT_FALSE(TimerAction1A.schedule(actionTicks, CompareAction::Set));

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, TimerAction1A.getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(11));

  // Schedule something with enough lead time
  startTicks = ExtTimer1.get();

  actionTicks = startTicks + 800ul;

  TEST_ASSERT_TRUE(TimerAction1A.schedule(actionTicks, CompareAction::Set));

  while (ExtTimer1.get() - startTicks < actionTicks - startTicks) {}

  TEST_ASSERT_EQUAL(TimerAction::Idle, TimerAction1A.getState());

  TEST_ASSERT_EQUAL(HIGH, digitalReadPWM(11));
}

void test_longmiss()
{
  // Force a miss by scheduling something further into the future than the system
  // register allows and turning off interrupts until after the action time
  noInterrupts();

  ticksExtraRange_t startTicks = ExtTimer1.get();

  const ticksExtraRange_t actionTicks = startTicks + ExtTimer1.getMaxSysTicks() + 1000ul;
  TEST_ASSERT_TRUE(TimerAction1A.schedule(actionTicks, CompareAction::Set));

  while (ExtTimer1.get() - startTicks < actionTicks - startTicks) {}
  
  // Now that the action time has passed, enable interrupts and let it process
  interrupts();
  while (ExtTimer1.get() - startTicks < actionTicks + ExtTimer1.getMaxSysTicks() + 1000ul - startTicks) {}

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, TimerAction1A.getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(11));
}

void test_origin()
{
  // Should miss since there isn't enough lead time
  ticksExtraRange_t originTicks = ExtTimer1.get();

  ticksExtraRange_t actionTicks = originTicks;

  TEST_ASSERT_FALSE(TimerAction1A.schedule(actionTicks, CompareAction::Set, originTicks));

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, TimerAction1A.getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(11));

  // Should be scheduled since TimerAction thinks the action is in the distant future
  originTicks = ExtTimer1.get();

  actionTicks = originTicks - 1ul;

  TEST_ASSERT_TRUE(TimerAction1A.schedule(actionTicks, CompareAction::Set, originTicks));

  TEST_ASSERT_EQUAL(TimerAction::WaitingToSchedule, TimerAction1A.getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(11));
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  //delay(2000);
  delay(100);

  UNITY_BEGIN();    // IMPORTANT LINE!

  RUN_TEST(test_basic);
  RUN_TEST(test_timerOverflow);
  RUN_TEST(test_timerOverflowOrigin);
  RUN_TEST(test_longmiss);
  RUN_TEST(test_shortmiss);
  RUN_TEST(test_supershortmiss);
  RUN_TEST(test_origin);

  UNITY_END(); // stop unit testing
}

void loop() {
}

