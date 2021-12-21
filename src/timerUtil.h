// AVR timer utils
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

#ifndef TIMER_EXT_TIMER_UTIL_H_
#define TIMER_EXT_TIMER_UTIL_H_

#include <stdint.h>

#include "timerTypes.h"

enum class TimerClock { None, Clk, ClkDiv8, ClkDiv1024 /* TODO: implement others*/ };

enum class TimerMode { Normal /* TODO: implement others*/ };

void configureTimerClock(Timer timer, TimerClock clock);

void configureTimerMode(Timer timer, TimerMode mode);

struct TimerConfig
{
  uint8_t tccra;
  uint8_t tccrb;
};

TimerConfig getTimerConfig(Timer timer);
void restoreTimerConfig(Timer timer, TimerConfig config);

#endif // TIMER_EXT_TIMER_UTIL_H_
