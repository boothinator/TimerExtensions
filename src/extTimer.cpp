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

#include "avr/interrupt.h"
#include "assert.h"

#include "timerTypes.h"

#if !defined(USE_ARDUINO_TIMER0_OVERFLOW) || USE_ARDUINO_TIMER0_OVERFLOW

extern volatile unsigned long timer0_overflow_count;

#endif

ExtTimer::ExtTimer(volatile uint8_t *tcntl, volatile uint8_t *tcnth, volatile uint8_t *timsk,
    uint8_t toie, volatile uint8_t *tifr, uint8_t tov, uint8_t timer) :
  _tcntl(tcntl), _tcnth(tcnth), _timsk(timsk),
    _toie(toie), _tifr(tifr), _tov(tov), _timer(timer)
{
  assert(tcntl);

  *_timsk |= _BV(_toie);
}

ticksExtraRange_t ExtTimer::get() const
{
  // Prevent overflow ticks from incrementing and prevent TOV flag from clearing
  char prevSREG = SREG;
  cli();

  ticksExtraRange_t ovfTicks = getOverflowTicks();

  ticks16_t sys = getSysRange();

  uint8_t ovf = *_tifr & (1 << _tov);

  SREG = prevSREG; // restore interrupt state of the caller

  if (sys < (1UL << 15) && ovf > 0)
  {
    return sys + ovfTicks + (1UL << 16);
  }
  else 
  {
    return sys + ovfTicks;
  }
}

ticksExtraRange_t ExtTimer::extend(ticks16_t ticks) const
{
  ticksExtraRange_t extTicks = ticks + getOverflowTicks();

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

ticksExtraRange_t ExtTimer::extendTimeInPast(ticks16_t ticks) const
{
  ticksExtraRange_t extTicks = ticks + getOverflowTicks();

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

ticks16_t ExtTimer::getSysRange() const
{
  if (_tcnth)
  {
    // 16-bit counter
    ticks16_t low, high;

    char prevSREG = SREG;
    cli();

    // Follow correct 16-bit register access rules by loading the low register first
    low = *_tcntl;
    high = *_tcnth;

    SREG = prevSREG; // restore interrupt state of the caller

    return low + (high << 8);
  }
  else
  {
    // 8-bit counter
    return *_tcntl;
  }
}

uint32_t ExtTimer::getOverflowCount() const
{
  ticksExtraRange_t tmp;
  
#ifdef timer0_overflow_count
  if (TIMER0 == _timer)
  {
    char prevSREG = SREG;
    cli();

    tmp = timer0_overflow_count;

    SREG = prevSREG; // restore interrupt state of the caller

    return tmp;
  }
#endif

  tmp = getOverflowTicks();

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
  cli();

#ifdef timer0_overflow_count
  if (TIMER0 == _timer)
  {
    timer0_overflow_count = 0;
  }
  else
  {
    _overflowTicks = 0;
  }
#else
  _overflowTicks = 0;
#endif

  SREG = prevSREG; // restore interrupt state of the caller
}

int ExtTimer::getTimer()
{
  return _timer;
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

ticksExtraRange_t ExtTimer::getOverflowTicks() const
{
  ticksExtraRange_t tmp;

  char prevSREG = SREG;
  cli();

#ifdef timer0_overflow_count
  if (TIMER0 == _timer)
  {
    tmp = timer0_overflow_count << 8;
  }
  else
  {
    tmp = _overflowTicks;
  }
#else
  tmp = _overflowTicks;
#endif

  SREG = prevSREG; // restore interrupt state of the caller

  return tmp;
}
