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

#include "extComp.h"

#include "avr/interrupt.h"
#include "assert.h"

ExtComp::ExtComp(ExtTimer *timer) 
{
  _timer = timer;
}

void ExtComp::scheduleEvent(ticksExtraRange_t ticks, callback_t cb)
{
  char prevSREG = SREG;
  cli();

  _ticks = ticks;
  _cb = cb;

  SREG = prevSREG; // restore interrupt state of the caller
}

void ExtComp::processCompareEvent()
{
  if (_cb && _timer->extendTimeInPast((ticks16_t)_ticks) == _ticks)
  {
    _cb();
    _cb = nullptr;
  }
}