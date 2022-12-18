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


void TimerAction::schedule(ticksExtraRange_t actionTicks, CompareAction action,
    TimerActionCallback cb, void *cbData)
{
  ticksExtraRange_t curTicks = _extTimer->get();

  _cb = cb;
  _cbData = cbData;
  _prevTicks = curTicks;
  _state = Scheduled;
  _action = action;
  _actionTicks = actionTicks;
  setOutputCompareTicks(_timer, static_cast<uint16_t>(actionTicks));
  *_timsk |= (1 << _ocie);
  tryScheduleSysRange(curTicks);
}

void TimerAction::tryScheduleSysRange(ticksExtraRange_t curTicks)
{
  uint32_t actionTicksDiff = _actionTicks - curTicks;

  // Is the event in time range?
  if (actionTicksDiff < _extTimer->getMaxSysTicks())
  {
    setOutputCompareAction(_timer, _action);
  }
  else
  {
    // Leave in previous state
  }
}

void TimerAction::processInterrupt()
{
  const ticksExtraRange_t curTicks = _extTimer->get();

  // The action is now in the past
  if (curTicks - _prevTicks > _actionTicks - _prevTicks)
  {
    // Disable interrupt
    *_timsk &= ~(1 << _ocie);

    // Check for miss
    if (curTicks - _actionTicks > _extTimer->getMaxSysTicks())
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
  }
  else
  {
    tryScheduleSysRange(curTicks);
  }

  _prevTicks = curTicks;
}

TimerAction::State TimerAction::getState() const
{
  return _state;
}

ticksExtraRange_t TimerAction::getActionTicks() const
{
  return _actionTicks;
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