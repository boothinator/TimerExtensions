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

#ifndef TIMER_EXT_TIMER_ACTION_H_
#define TIMER_EXT_TIMER_ACTION_H_

#include <stdint.h>

#include "extTimer.h"
#include "timerUtil.h"

class TimerAction {
public:
  enum State : uint8_t {Idle, WaitingToSchedule, Scheduled, MissedAction};

  typedef void (*TimerActionCallback) (TimerAction *, void *);

  TimerAction(int timer, ExtTimer *extTimer, uint8_t ocie, uint8_t ocf)
    : _timer{timer}, _extTimer{extTimer}, _ocie{ocie}, _ocf{ocf}
  {}

  bool schedule(ticksExtraRange_t actionTicks, CompareAction action,
      TimerActionCallback cb = nullptr, void *cbData = nullptr);
  bool schedule(ticksExtraRange_t actionTicks, CompareAction action, ticksExtraRange_t curTicks,
      TimerActionCallback cb = nullptr, void *cbData = nullptr);

  void processInterrupt();

  State getState() const;

  ticksExtraRange_t getActionTicks() const;

  ExtTimer *getExtTimer() const;

  int getTimer() const;

  CompareAction getAction();

private:
  int _timer;
  ExtTimer *_extTimer;
  
  ticksExtraRange_t _actionTicks;

  // Defines dividing line between near future (_originTicks < _extTimer.getMaxSysTicks())
  // and far future (_originTicks > _extTimer.getMaxSysTicks())
  //
  // Defines an origin for determining when an action should have happened
  // (curTicks - _originTicks > actionTicks - _originTicks)
  //
  // Defines what it means to miss an action
  // (curTicks - _originTicks > actionTicks - _originTicks && hit == false)
  ticksExtraRange_t _originTicks;

  uint8_t _ocie;
  uint8_t _ocf;
  
  CompareAction _action;

  TimerActionCallback _cb = nullptr;
  void *_cbData = nullptr;

  State _state = Idle;

  void tryScheduleSysRange(ticksExtraRange_t curTicks);
  bool tryProcessActionInPast(ticksExtraRange_t curTicks);
};

// TODO: others

#ifdef HAVE_TCNT1

extern TimerAction TimerAction1A;

#endif // HAVE_TCNT1

#endif // TIMER_EXT_TIMER_ACTION_H_