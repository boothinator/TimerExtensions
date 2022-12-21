// Precise AVR pulse generation
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

#include "pulseGen.h"

#include "avr/interrupt.h"
#include "util/atomic.h"

#include "timerTypes.h"

namespace
{

void startTimerActionCallback(TimerAction *timerAction, void *data)
{
  PulseGen *pulseGen = static_cast<PulseGen *>(data);

  pulseGen->scheduleEndAction();
}

void endTimerActionCallback(TimerAction *timerAction, void *data)
{
  PulseGen *pulseGen = static_cast<PulseGen *>(data);

  pulseGen->finish();
}


} // namespace

bool PulseGen::schedule(ticksExtraRange_t start, ticksExtraRange_t end)
{
  bool scheduled = _timerAction->schedule(start, CompareAction::Set, startTimerActionCallback, this);

  if (!scheduled)
  {
    _state = MissedStart;
      
    if (_cb)
    {
      _cb(this, _cbData);
    }
    return false;
  }

  _state = ScheduledStart;

  _start = start;
  _end = end;

  return true;
}

ticksExtraRange_t PulseGen::getStart() const
{
  return _start;
}

ticksExtraRange_t PulseGen::getEnd() const
{
  return _end;
}


void PulseGen::setStateChangeCallback(stateChangeCallback_t _cb, const void *_cbData)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    this->_cb = _cb;
    this->_cbData = _cbData;
  }
}

bool PulseGen::cancel()
{
  if (ScheduledStart == _state)
  {
    return cancelStartOrEnd();
  }
  else
  {
    // Use cancelEnd() to cancel just the end
    return false;
  }
}


bool PulseGen::cancelEnd()
{
  if (ScheduledEnd == _state)
  {
    return cancelStartOrEnd();
  }
  else
  {
    return false;
  }
}

bool PulseGen::cancelStartOrEnd()
{
  bool cancelled = _timerAction->cancel();

  if (!cancelled)
  {
    _state = Idle;
    return false;
  }

  return true;
}

PulseGen::State PulseGen::getState() const
{
  return _state;
}

TimerAction *PulseGen::getTimerAction() const
{
  return _timerAction;
}

void PulseGen::scheduleEndAction()
{
  TimerAction::State timerActionState = _timerAction->getState();

  if (TimerAction::Idle == timerActionState)
  {
    bool scheduled = _timerAction->schedule(_end, CompareAction::Clear, _start,
      endTimerActionCallback, this);
    
    if (scheduled)
    {
      _state = ScheduledEnd;

      if (_cb)
      {
        _cb(this, _cbData);
      }
    }
    else
    {
      _state = MissedEnd;
      
      if (_cb)
      {
        _cb(this, _cbData);
      }
    }
  }
  else if (TimerAction::MissedAction == timerActionState)
  {
    _state = MissedStart;

    if (_cb)
    {
      _cb(this, _cbData);
    }
  }
}

void PulseGen::finish()
{
  TimerAction::State timerActionState = _timerAction->getState();

  if (TimerAction::Idle == timerActionState)
  {
    _state = Idle;

    if (_cb)
    {
      _cb(this, _cbData);
    }
  }
  else if (TimerAction::MissedAction == timerActionState)
  {
    _state = MissedEnd;

    if (_cb)
    {
      _cb(this, _cbData);
    }
  }
}