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
  if (Scheduled == _state || WaitingToSchedule == _state)
  {
    // Disable interrupt
    *_extTimer->getTIMSK() &= ~(1 << _ocie);
  }

  CompareAction prevCompareAction = getOutputCompareAction(_timer);

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

  // Set the action and the action time
  tryScheduleSysRange(_extTimer->get());

  bool successful = true;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Setting OCR could fire the interrupt, so process first
    setOutputCompareTicks(_timer, static_cast<uint16_t>(actionTicks));

      ticksExtraRange_t ticksAfterSet = _extTimer->get();

      // Are we now after the action time?
      bool shouldHaveHit = ticksAfterSet - _originTicks > _actionTicks - _originTicks;

    if (shouldHaveHit)
    {
      bool didHit = *_extTimer->getTIFR() & (1 << _ocf);

      if (!didHit)
      {
        setOutputCompareAction(_timer, prevCompareAction);
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

bool TimerAction::schedule(ticksExtraRange_t actionTicks, CompareAction action,
    TimerActionCallback cb, void *cbData)
{
  ticksExtraRange_t curTicks = _extTimer->get();

  return schedule(actionTicks, action, curTicks, cb, cbData);
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