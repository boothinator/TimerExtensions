// Extended Range AVR Timer
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

#ifndef TIMER_EXT_EXT_TIMER_H_
#define TIMER_EXT_EXT_TIMER_H_

#include "timerTypes.h"

// Extend the range of 16-bit AVR timers
class ExtTimer
{
public:
  ExtTimer(volatile uint16_t *_tcnt2);
  
  const ticksExtraRange_t get();

  // Extend the range of the passed-in ticks
  // Assumes that ticks is in the future
  const ticksExtraRange_t extend(ticksSysRange_t ticks);

  const ticksExtraRange_t extendTimeInPast(ticksSysRange_t ticks);

  const ticksSysRange_t getSysRange();

  const uint16_t getOverflowCount();
  void resetOverflowCount();

  void processOverflow();

private:
  volatile uint16_t _overflowCount;
  volatile uint16_t *_tcnt;
};

#endif // TIMER_EXT_EXT_TIMER_H_