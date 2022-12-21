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

#ifndef TIMER_EXT_PULSE_H_
#define TIMER_EXT_PULSE_H_

#include "timerTypes.h"
#include "timerAction.h"

class PulseGen
{
public:
  typedef void (*stateChangeCallback_t)(PulseGen *pulse, const void *data);

  PulseGen(TimerAction &timerAction, stateChangeCallback_t cb = nullptr, const void *cbData = nullptr)
    : _timerAction{&timerAction}, _cb{cb}, _cbData{cbData}
  {}

  bool schedule(ticksExtraRange_t start, ticksExtraRange_t end);

  ticksExtraRange_t getStart() const;
  ticksExtraRange_t getEnd() const;

  bool cancel();
  bool cancelEnd();
  bool cancelStartOrEnd();

  enum State : uint8_t {Idle, ScheduledStart, ScheduledEnd, MissedStart, MissedEnd};

  State getState() const;

  TimerAction *getTimerAction() const;

  void setStateChangeCallback(stateChangeCallback_t _cb, const void *_cbData = nullptr);

  void scheduleEndAction();
  void finish();

private:
  TimerAction *_timerAction;

  stateChangeCallback_t _cb = nullptr;
  const void *_cbData = nullptr;

  ticksExtraRange_t _start;
  ticksExtraRange_t _end;

  volatile State _state = State::Idle;
};


#endif // TIMER_EXT_PULSE_H_
