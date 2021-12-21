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

#include "extTimer.h"

#include "assert.h"

#include "timerTypes.h"

ExtTimer::ExtTimer(volatile uint16_t *tcnt)
{
  assert(tcnt);

  this->_tcnt = tcnt;
  _overflowCount = 0;
}

const ticksExtraRange_t ExtTimer::get()
{
  char prevSREG = SREG;
  noInterrupts();

  ticksExtraRange_t tmp = *_tcnt + ((ticksExtraRange_t)_overflowCount << 16);

  SREG = prevSREG; // restore interrupt state of the caller

  return tmp;
}

const ticksExtraRange_t ExtTimer::extend(ticksSysRange_t ticks)
{
  ticksExtraRange_t extTicks = ticks + ((ticksExtraRange_t)_overflowCount << 16);

  // Add another overflow if the ticks is before tcnt.
  // We're assuming that ticks is in the future, so so tcnt needs to roll over to get there.
  if (ticks < getSysRange())
  {
    extTicks += (1UL << 16);
  }

  return extTicks;
}

const ticksExtraRange_t ExtTimer::extendTimeInPast(ticksSysRange_t ticks)
{
  ticksExtraRange_t extTicks = ticks + ((ticksExtraRange_t)_overflowCount << 16);

  // Subtract another overflow if the ticks is after tcnt.
  // We're assuming that ticks is in the past, so so tcnt would have rolled over to get here.
  if (getSysRange() < ticks)
  {
    extTicks -= (1UL << 16);
  }

  return extTicks;
}

const ticksSysRange_t ExtTimer::getSysRange()
{
  char prevSREG = SREG;
  noInterrupts();

  ticksExtraRange_t tmp = *_tcnt;

  SREG = prevSREG; // restore interrupt state of the caller

  return tmp;
}

const uint16_t ExtTimer::getOverflowCount()
{
  char prevSREG = SREG;
  noInterrupts();

  ticksExtraRange_t tmp = _overflowCount;

  SREG = prevSREG; // restore interrupt state of the caller

  return tmp;
}

void ExtTimer::resetOverflowCount()
{
  _overflowCount = 0;
}

void ExtTimer::processOverflow()
{
  _overflowCount++;
}