// TimerAction
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

#include "timerAction.h"

#include <util/atomic.h>

bool TimerAction::schedule(ticksExtraRange_t actionTicks, CompareAction action,
    ticksExtraRange_t originTicks, TimerActionCallback cb, void *cbData)
{
  _prevCompareAction = getOutputCompareAction(_timer);

  _cb = cb;
  _cbData = cbData;
  _originTicks = originTicks;
  _state = WaitingToSchedule;
  _action = action;
  _actionTicks = actionTicks;

  // Set OCR to a known value in the past and clear any pending int flag
  // Ensures that we have plety of time to set OCR without accidentally
  // matching
  setOutputCompareTicks(_timer, static_cast<uint16_t>(originTicks - 1));
  *_extTimer->getTIFR() = (1 << _ocf);
  
  // Enable the interrupt
  *_extTimer->getTIMSK() |= (1 << _ocie);

  // Set the action
  tryScheduleSysRange(_extTimer->get());

  bool successful = true;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Set the action time and figure out if it should have hit
    setOutputCompareTicks(_timer, static_cast<uint16_t>(actionTicks));

    ticksExtraRange_t ticksAfterSet = _extTimer->get();

    // Are we now after the action time?
    bool shouldHaveHit = ticksAfterSet - _originTicks > _actionTicks - _originTicks;

    if (shouldHaveHit)
    {
      bool didHit = *_extTimer->getTIFR() & (1 << _ocf);

      if (!didHit)
      {
        setOutputCompareAction(_timer, _prevCompareAction);
        _state = MissedAction;
        successful = false;

        if (_cb) {
          _cb(this, _cbData);
        }
      }
    }
  }

  return successful;
}

ticksExtraRange_t TimerAction::getBackdateTicks()
{
  static constexpr ticksExtraRange_t backdateClockCycles = 8192ul;

  TimerClock clk = getTimerClock(_timer);

  switch(clk)
  {
    case TimerClock::Clk:
      return backdateClockCycles / clockCyclesPerTick(TimerClock::Clk);
    case TimerClock::ClkDiv8:
      return backdateClockCycles / clockCyclesPerTick(TimerClock::ClkDiv8);
    case TimerClock::ClkDiv32:
      return backdateClockCycles / clockCyclesPerTick(TimerClock::ClkDiv32);
    case TimerClock::ClkDiv64:
      return backdateClockCycles / clockCyclesPerTick(TimerClock::ClkDiv64);
    case TimerClock::ClkDiv128:
      return backdateClockCycles / clockCyclesPerTick(TimerClock::ClkDiv128);
    case TimerClock::ClkDiv256:
      return backdateClockCycles / clockCyclesPerTick(TimerClock::ClkDiv256);
    case TimerClock::ClkDiv1024:
      return backdateClockCycles / clockCyclesPerTick(TimerClock::ClkDiv1024);
    default:
      return 0;
  }
}

bool TimerAction::schedule(ticksExtraRange_t actionTicks, CompareAction action,
    TimerActionCallback cb, void *cbData)
{
  ticksExtraRange_t originTicks = _extTimer->get() - getBackdateTicks();

  return schedule(actionTicks, action, originTicks, cb, cbData);
}

void TimerAction::tryScheduleSysRange(ticksExtraRange_t curTicks)
{
  uint32_t actionTicksDiff = _actionTicks - curTicks;

  // Is the event in time range?
  if (actionTicksDiff < _extTimer->getMaxSysTicks())
  {
    setOutputCompareAction(_timer, _action);
    _state = Scheduled;
  }
  else
  {
    // Leave in previous state
    _state = WaitingToSchedule;
  }
}

bool TimerAction::cancel()
{
  if (WaitingToSchedule == _state)
  {
    // Disable the interrupt and set previous compare action
    *_extTimer->getTIMSK() &= ~(1 << _ocie);
    setOutputCompareAction(_timer, _prevCompareAction);
    return true;
  }
  else if (Scheduled == _state)
  {
    ticks16_t recentPastTime = _extTimer->getSysRange() - 1;

    bool didHit;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      // Set the action time to a time in the recent past so it won't hit
      setOutputCompareTicks(_timer, recentPastTime);

      didHit = *_extTimer->getTIFR() & (1 << _ocf);
    }

    // Disable the interrupt and set previous compare action
    *_extTimer->getTIMSK() &= ~(1 << _ocie);
    setOutputCompareAction(_timer, _prevCompareAction);

    if (didHit)
    {
      // We missed the chance to cancel
      return false;
    }
    else
    {
      // Cancelled successfully
      return true;
    }
  }
  else
  {
    // Totally missed
    return false;
  }
}

bool TimerAction::tryProcessActionInPast(ticksExtraRange_t curTicks)
{
  // The action is now in the past
  if (curTicks - _originTicks > _actionTicks - _originTicks)
  {
    // Disable interrupt
    *_extTimer->getTIMSK() &= ~(1 << _ocie);

    // Check for miss
    if (WaitingToSchedule == _state)
    {
      _state = MissedAction;
    }
    else
    {
      _state = Idle;
    }

    if (_cb) {
      _cb(this, _cbData);
    }

    return true;
  }
  else
  {
    return false;
  }
}

void TimerAction::processInterrupt()
{
  const ticksExtraRange_t curTicks = _extTimer->get();

  // The action is now in the past
  if (!tryProcessActionInPast(curTicks))
  {
    tryScheduleSysRange(curTicks);
  }
}

TimerAction::State TimerAction::getState() const
{
  return _state;
}

ticksExtraRange_t TimerAction::getActionTicks() const
{
  return _actionTicks;
}

ticksExtraRange_t TimerAction::getOriginTicks() const
{
  return _originTicks;
}

ExtTimer *TimerAction::getExtTimer() const
{
  return _extTimer;
}

int TimerAction::getTimer() const
{
  return _timer;
}

CompareAction TimerAction::getAction()
{
  return _action;
}