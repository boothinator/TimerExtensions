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

ExtTimer::ExtTimer(volatile uint8_t *tcntl, volatile uint8_t *tcnth)
{
  assert(tcntl);

  this->_tcntl = tcntl;
  this->_tcnth = tcnth;
}

const ticksExtraRange_t ExtTimer::get()
{
  ticksExtraRange_t tmp = getSysRange();
  
  tmp += _overflowTicks;

  return tmp;
}

const ticksExtraRange_t ExtTimer::extend(ticks16_t ticks)
{
  ticksExtraRange_t extTicks = ticks + _overflowTicks;

  // Add another overflow if the ticks is before tcnt.
  // We're assuming that ticks is in the future, so so tcnt needs to roll over to get there.
  if (ticks < getSysRange())
  {
    if (_tcnth)
    {
      // 16-bit timer
      extTicks += (1UL << 16);
    }
    else
    {
      // 8-bit timer
      extTicks += (1UL << 8);
    }
  }

  return extTicks;
}

const ticksExtraRange_t ExtTimer::extendTimeInPast(ticks16_t ticks)
{
  ticksExtraRange_t extTicks = ticks + _overflowTicks;

  // Subtract another overflow if the ticks is after tcnt.
  // We're assuming that ticks is in the past, so so tcnt would have rolled over to get here.
  if (getSysRange() < ticks)
  {
    if (_tcnth)
    {
      // 16-bit timer
      extTicks -= (1UL << 16);
    }
    else
    {
      // 8-bit timer
      extTicks -= (1UL << 8);
    }
  }

  return extTicks;
}

const ticks16_t ExtTimer::getSysRange()
{
  char prevSREG = SREG;
  noInterrupts();

  ticksExtraRange_t tmp = *_tcntl;

  // Follow correct 16-bit register access rules by loading the low register first
  if (_tcnth)
  {
    tmp += (*_tcnth) << 8;
  }

  SREG = prevSREG; // restore interrupt state of the caller

  return tmp;
}

const uint16_t ExtTimer::getOverflowCount()
{
  char prevSREG = SREG;
  noInterrupts();

  ticksExtraRange_t tmp = _overflowTicks;

  SREG = prevSREG; // restore interrupt state of the caller

  if (_tcnth)
  {
    // Discard bottom 16 bits for 16-bit timers
    tmp >>= 16;
  }
  else
  {
    // Discard bottom 8 bits for 8-bit timers
    tmp >>= 8;
  }

  return tmp;
}

void ExtTimer::resetOverflowCount()
{
  char prevSREG = SREG;
  noInterrupts();

  _overflowTicks = 0;

  SREG = prevSREG; // restore interrupt state of the caller
}

void ExtTimer::processOverflow()
{
  if (_tcnth)
  {
    // add 65536 to the overflow ticks counter on overflow for 16-bit timers
    _overflowTicks += (1UL << 16);
  }
  else
  {
    // add 256 to the overflow ticks counter on overflow for 8-bit timers
    _overflowTicks += (1UL << 8);
  }
}