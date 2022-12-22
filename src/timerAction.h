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

  void configure(TimerClock clock);

  ticksExtraRange_t millisecondsToTicks(uint32_t milliseconds);
  ticksExtraRange_t microsecondsToTicks(uint32_t microseconds);
  ticksExtraRange_t getNow();

  bool schedule(ticksExtraRange_t actionTicks, CompareAction action,
      TimerActionCallback cb = nullptr, void *cbData = nullptr);
  bool schedule(ticksExtraRange_t actionTicks, CompareAction action, ticksExtraRange_t originTicks,
      TimerActionCallback cb = nullptr, void *cbData = nullptr);
  bool schedule(ticksExtraRange_t actionTicks, ticksExtraRange_t originTicks,
      TimerActionCallback cb = nullptr, void *cbData = nullptr);
  bool schedule(ticksExtraRange_t actionTicks,
      TimerActionCallback cb = nullptr, void *cbData = nullptr);

  bool cancel();

  void processInterrupt();

  State getState() const;

  ticksExtraRange_t getActionTicks() const;
  ticksExtraRange_t getOriginTicks() const;

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

  volatile State _state = Idle;

  CompareAction _prevCompareAction;

  void tryScheduleSysRange(ticksExtraRange_t curTicks);
  bool tryProcessActionInPast(ticksExtraRange_t curTicks);
  ticksExtraRange_t getBackdateTicks();
};

#ifdef HAVE_TCNT0

extern TimerAction TimerAction0A;
extern TimerAction TimerAction0B;

#endif // HAVE_TCNT0

#ifdef HAVE_TCNT1

extern TimerAction TimerAction1A;
extern TimerAction TimerAction1B;
#ifdef OCR1C
extern TimerAction TimerAction1C;
#endif // OCR1C

#endif // HAVE_TCNT1

#ifdef HAVE_TCNT2

extern TimerAction TimerAction2A;
extern TimerAction TimerAction2B;

#endif // HAVE_TCNT2

#ifdef HAVE_TCNT3

extern TimerAction TimerAction3A;
extern TimerAction TimerAction3B;
extern TimerAction TimerAction3C;

#endif // HAVE_TCNT3

#ifdef HAVE_TCNT4

extern TimerAction TimerAction4A;
extern TimerAction TimerAction4B;
extern TimerAction TimerAction4C;

#endif // HAVE_TCNT4

#ifdef HAVE_TCNT5

extern TimerAction TimerAction5A;
extern TimerAction TimerAction5B;
extern TimerAction TimerAction5C;

#endif // HAVE_TCNT5



#ifdef ARDUINO_AVR_UNO

#define TimerActionPin6 TimerAction0A
#define TimerActionPin5 TimerAction0B

#define TimerActionPin9 TimerAction1A
#define TimerActionPin10 TimerAction1B

#define TimerActionPin11 TimerAction2A
#define TimerActionPin3 TimerAction2B

#endif // ARDUINO_AVR_UNO



#ifdef ARDUINO_AVR_MEGA2560

#define TimerActionPin13Timer0 TimerAction0A
#define TimerActionPin4 TimerAction0B

#define TimerActionPin11 TimerAction1A
#define TimerActionPin12 TimerAction1B
#define TimerActionPin13Timer1 TimerAction1C

#define TimerActionPin10 TimerAction2A
#define TimerActionPin9 TimerAction2B

#define TimerActionPin5 TimerAction3A
#define TimerActionPin2 TimerAction3B
#define TimerActionPin3 TimerAction3C

#define TimerActionPin6 TimerAction4A
#define TimerActionPin7 TimerAction4B
#define TimerActionPin8 TimerAction4C

#define TimerActionPin46 TimerAction5A
#define TimerActionPin45 TimerAction5B
#define TimerActionPin44 TimerAction5C

#endif // ARDUINO_AVR_MEGA2560


#endif // TIMER_EXT_TIMER_ACTION_H_