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
  enum State : uint8_t {Idle, Scheduled, MissedAction};

  typedef void (*TimerActionCallback) (TimerAction *, void *);

  TimerAction(int timer, ExtTimer *extTimer, volatile uint8_t *timsk, uint8_t ocie)
    : _timer{timer}, _extTimer{extTimer}, _timsk{timsk}, _ocie{ocie}
  {}

  void schedule(ticksExtraRange_t actionTicks, CompareAction action,
      TimerActionCallback cb, void *cbData = nullptr);

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
  ticksExtraRange_t _prevTicks;

  volatile uint8_t *_timsk;
  uint8_t _ocie;
  
  CompareAction _action;

  TimerActionCallback _cb = nullptr;
  void *_cbData = nullptr;

  State _state = Idle;

  void tryScheduleSysRange(ticksExtraRange_t curTicks);
};

#endif // TIMER_EXT_TIMER_ACTION_H_