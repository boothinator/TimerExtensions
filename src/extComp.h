// Extended Range AVR Compare
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

#ifndef TIMER_EXT_EXT_COMP_H_
#define TIMER_EXT_EXT_COMP_H_

#include <Arduino.h>

#include "timerTypes.h"
#include "extTimer.h"

class ExtComp
{
public:
  typedef void (*callback_t)();

  ExtComp(ExtTimer *timer);

  void scheduleEvent(ticksExtraRange_t ticks, callback_t cb);

  void processCompareEvent();

private:
  ticksExtraRange_t _ticks = 0;
  ExtTimer *_timer;
  volatile callback_t _cb = nullptr;
};

#endif // TIMER_EXT_EXT_COMP_H_