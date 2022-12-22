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

volatile int cbCallCount = 0;

int pin = 11;
ExtTimer *extTimer = &ExtTimer1;
TimerAction *timerAction = &TimerAction1A;

void setUp(void) {
  pinMode(pin, OUTPUT);

  extTimer->configure(TimerClock::Clk);

  cbCallCount = 0;
}

void tearDown(void) {
  setOutputCompareAction(timerAction->getTimer(), CompareAction::Nothing);
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

  int pinStartState = digitalReadPWM(pin);

  ticksExtraRange_t startTicks = extTimer->get();

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, action));

  bool timedOut = false;

  while ((
      timerAction->getState() == TimerAction::Scheduled
      || timerAction->getState() == TimerAction::WaitingToSchedule
    )
    && !timedOut)
  {
    timedOut = extTimer->get() - startTicks > actionTicks - startTicks + 1000;
  }

  snprintf(message, MAX_MESSAGE_LEN, "scheduleAndTest() ActionTicks: %lx, CurTicks: %lx, OriginTicks: %lx, Action: %u, State: %u",
    actionTicks, extTimer->get(), timerAction->getOriginTicks(), action, timerAction->getState());

  TEST_ASSERT_FALSE_MESSAGE(timedOut, message);

  TEST_ASSERT_EQUAL_MESSAGE(TimerAction::Idle, timerAction->getState(), message);

  if (CompareAction::Set == action)
  {
    TEST_ASSERT_EQUAL_MESSAGE(HIGH, digitalReadPWM(pin), message);
  }
  else if (CompareAction::Clear == action)
  {
    TEST_ASSERT_EQUAL_MESSAGE(LOW, digitalReadPWM(pin), message);
  }
  else if (CompareAction::Toggle == action)
  {
    TEST_ASSERT_NOT_EQUAL_MESSAGE(pinStartState, digitalReadPWM(pin), message);
  }
}

void test_basic()
{
  scheduleAndTest(extTimer->get() + 10000ul, CompareAction::Set);
  /*scheduleAndTest(extTimer->get() + 10000ul, CompareAction::Clear);
  scheduleAndTest(extTimer->get() + 10000ul, CompareAction::Toggle);
  scheduleAndTest(extTimer->get() + 1000000ul, CompareAction::Set);
  scheduleAndTest(extTimer->get() + 1000000ul, CompareAction::Clear);
  scheduleAndTest(extTimer->get() + 1000000ul, CompareAction::Toggle);*/
}

void test_timerOverflow()
{
  const ticksExtraRange_t actionTicks = 100ul;
  extTimer->set(actionTicks - 2000ul);
  scheduleAndTest(actionTicks, CompareAction::Set);
}

void test_timerOverflowOrigin()
{
  ticksExtraRange_t originTicks = UINT32_MAX - 10000ul;
  extTimer->set(originTicks);

  ticksExtraRange_t actionTicks = 100ul;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, originTicks));

  while (extTimer->get() - originTicks < actionTicks - originTicks) {}

  TEST_ASSERT_EQUAL(TimerAction::Idle, timerAction->getState());

  TEST_ASSERT_EQUAL(HIGH, digitalReadPWM(pin));
}

void test_supershortmiss()
{
  // Schedule something without enough lead time
  ticksExtraRange_t startTicks = extTimer->get();

  ticksExtraRange_t actionTicks = startTicks + 100ul;

  TEST_ASSERT_FALSE(timerAction->schedule(actionTicks, CompareAction::Set, startTicks));

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, timerAction->getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));
}

void test_pastmiss()
{
  // Schedule something without enough lead time
  ticksExtraRange_t startTicks = extTimer->get() - 1000;

  ticksExtraRange_t actionTicks = startTicks + 100ul;

  TEST_ASSERT_FALSE(timerAction->schedule(actionTicks, CompareAction::Set, startTicks));

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, timerAction->getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));
}

void test_shortmiss()
{
  // Schedule something without enough lead time
  ticksExtraRange_t startTicks = extTimer->get();

  ticksExtraRange_t actionTicks = startTicks + 200ul;

  TEST_ASSERT_FALSE(timerAction->schedule(actionTicks, CompareAction::Set));

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, timerAction->getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));

  // Schedule something with enough lead time
  startTicks = extTimer->get();

  actionTicks = startTicks + 1000ul;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set));

  while (extTimer->get() - startTicks < actionTicks - startTicks) {}

  TEST_ASSERT_EQUAL(TimerAction::Idle, timerAction->getState());

  TEST_ASSERT_EQUAL(HIGH, digitalReadPWM(pin));
}

void test_longmiss()
{
  // Force a miss by scheduling something further into the future than the system
  // register allows and turning off interrupts until after the action time
  noInterrupts();

  ticksExtraRange_t startTicks = extTimer->get();

  const ticksExtraRange_t actionTicks = startTicks + extTimer->getMaxSysTicks() + 1000ul;
  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set));

  while (extTimer->get() - startTicks < actionTicks - startTicks) {}
  
  // Now that the action time has passed, enable interrupts and let it process
  interrupts();
  while (extTimer->get() - startTicks < actionTicks + extTimer->getMaxSysTicks() + 1000ul - startTicks) {}

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, timerAction->getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));
}

void test_origin()
{
  // Should miss since there isn't enough lead time
  ticksExtraRange_t originTicks = extTimer->get();

  ticksExtraRange_t actionTicks = originTicks;

  TEST_ASSERT_FALSE(timerAction->schedule(actionTicks, CompareAction::Set, originTicks));

  TEST_ASSERT_EQUAL(TimerAction::MissedAction, timerAction->getState());

  while (extTimer->get() - originTicks < actionTicks - originTicks) {}

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));

  // Should be scheduled since TimerAction thinks the action is in the distant future
  originTicks = extTimer->get();

  actionTicks = originTicks - 1ul;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, originTicks));

  TEST_ASSERT_EQUAL(TimerAction::WaitingToSchedule, timerAction->getState());

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));
}

void test_cancel_success()
{
  ticksExtraRange_t originTicks = extTimer->get();

  ticksExtraRange_t actionTicks = originTicks + 2000;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, originTicks));

  TEST_ASSERT_TRUE(timerAction->cancel());

  while (extTimer->get() - originTicks < actionTicks - originTicks) {}

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));
}

void test_cancel_long_success()
{
  ticksExtraRange_t originTicks = extTimer->get();

  ticksExtraRange_t actionTicks = originTicks + 200000;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, originTicks));

  TEST_ASSERT_TRUE(timerAction->cancel());

  while (extTimer->get() - originTicks < actionTicks - originTicks) {}

  TEST_ASSERT_EQUAL(LOW, digitalReadPWM(pin));
}

void test_cancel_failure()
{
  ticksExtraRange_t originTicks = extTimer->get();

  ticksExtraRange_t actionTicks = originTicks + 800;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, originTicks));

  TEST_ASSERT_FALSE(timerAction->cancel());

  while (extTimer->get() - originTicks < actionTicks - originTicks) {}

  TEST_ASSERT_EQUAL(HIGH, digitalReadPWM(pin));
}

void cb(TimerAction *timerAction, void *data)
{
  cbCallCount++;
}

void test_cb()
{
  ticksExtraRange_t actionTicks = extTimer->get() + 1000;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, cb));

  while (extTimer->get() < actionTicks + 100) {}

  TEST_ASSERT_EQUAL(1, cbCallCount);
}

void test_cbMiss()
{
  ticksExtraRange_t actionTicks = extTimer->get();

  TEST_ASSERT_FALSE(timerAction->schedule(actionTicks, CompareAction::Set, cb));

  TEST_ASSERT_EQUAL(1, cbCallCount);
}

void cbChained(TimerAction *timerAction, void *data)
{
  ticksExtraRange_t actionTicks = extTimer->get() + 1000;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, cb));
}

void cbChained2(TimerAction *timerAction, void *data)
{
  ticksExtraRange_t actionTicks = extTimer->get() + 1000;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, cbChained));
}

void test_cbChained()
{
  ticksExtraRange_t actionTicks = extTimer->get() + 1000;

  TEST_ASSERT_TRUE(timerAction->schedule(actionTicks, CompareAction::Set, cbChained2));

  while (extTimer->get() < actionTicks + 3000) {}

  TEST_ASSERT_EQUAL(1, cbCallCount);
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();    // IMPORTANT LINE!

  pin = 11;
  extTimer = &ExtTimer1;
  timerAction = &TimerAction1A;

  RUN_TEST(test_basic);
  RUN_TEST(test_timerOverflow);
  RUN_TEST(test_timerOverflowOrigin);
  RUN_TEST(test_longmiss);
  RUN_TEST(test_shortmiss);
  RUN_TEST(test_supershortmiss);
  RUN_TEST(test_pastmiss);
  RUN_TEST(test_origin);
  RUN_TEST(test_cancel_success);
  RUN_TEST(test_cancel_long_success);
  RUN_TEST(test_cancel_failure);
  RUN_TEST(test_cb);
  RUN_TEST(test_cbMiss);
  RUN_TEST(test_cbChained);

  pin = 6;
  extTimer = &ExtTimer0;
  timerAction = &TimerAction0A;

  /*RUN_TEST(test_basic);
  RUN_TEST(test_timerOverflow);
  RUN_TEST(test_timerOverflowOrigin);
  RUN_TEST(test_longmiss);
  RUN_TEST(test_shortmiss);
  RUN_TEST(test_supershortmiss);
  RUN_TEST(test_pastmiss);
  RUN_TEST(test_origin);
  RUN_TEST(test_cancel_success);
  RUN_TEST(test_cancel_long_success);
  RUN_TEST(test_cancel_failure);
  RUN_TEST(test_cb);
  RUN_TEST(test_cbMiss);
  RUN_TEST(test_cbChained);*/

  pin = 8;
  extTimer = &ExtTimer2;
  timerAction = &TimerAction2A;

  /*RUN_TEST(test_basic);
  RUN_TEST(test_timerOverflow);
  RUN_TEST(test_timerOverflowOrigin);
  RUN_TEST(test_longmiss);
  RUN_TEST(test_shortmiss);
  RUN_TEST(test_supershortmiss);
  RUN_TEST(test_pastmiss);
  RUN_TEST(test_origin);
  RUN_TEST(test_cancel_success);
  RUN_TEST(test_cancel_long_success);
  RUN_TEST(test_cancel_failure);
  RUN_TEST(test_cb);
  RUN_TEST(test_cbMiss);
  RUN_TEST(test_cbChained);*/

  UNITY_END(); // stop unit testing
}

void loop() {
}

