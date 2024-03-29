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
#include "util/atomic.h"

#include "timerTypes.h"

#ifndef IMPLEMENT_TIMER0_OVERFLOW
#define IMPLEMENT_TIMER0_OVERFLOW 0
#endif

#define USE_ARDUINO_TIMER0_OVERFLOW (!IMPLEMENT_TIMER0_OVERFLOW)

#if USE_ARDUINO_TIMER0_OVERFLOW

extern volatile unsigned long timer0_overflow_count;

#endif

ExtTimer::ExtTimer(volatile uint8_t *tcntl, volatile uint8_t *tcnth, volatile uint8_t *timsk,
    uint8_t toie, volatile uint8_t *tifr, uint8_t tov, uint8_t timer) :
  _tcntl(tcntl), _tcnth(tcnth), _timsk(timsk),
    _toie(toie), _tifr(tifr), _tov(tov), _timer(timer)
{
  *_timsk |= _BV(_toie);
}
  
void ExtTimer::configure(TimerClock clock)
{
  setTimerClock(_timer, clock);
  setTimerMode(_timer, TimerMode::Normal);
}

ticksExtraRange_t ExtTimer::get() const
{
  ticksExtraRange_t ovfTicks;
  ticksExtraRange_t sys;
  uint8_t ovf;
  
  // Prevent overflow ticks from incrementing and prevent TOV flag from clearing
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    ovfTicks = getOverflowTicksInternal();

    sys = getSysRange();

    ovf = *_tifr & (1 << _tov);
  }

  ticksExtraRange_t ticks = sys + ovfTicks;

  // Handle overflow only if the time overflowed recently
  // and the overflow flag is set
  if (ovf)
  {
    if (_tcnth)
    {
      // 16-bit timer
      if (sys < (1UL << 15))
      {
        ticks += (1UL << 16);
      }
    }
    else
    {
      // 8-bit timer
      if (sys < (1UL << 7))
      {
        ticks += (1UL << 8);
      }
    }
  }

  return ticks;
}

void ExtTimer::set(ticksExtraRange_t ticks)
{

  if (_tcnth)
  {
    // 16-bit timer

    uint8_t highByte = (ticks & 0x0000FF00) >> 8;

    // Ensure that TCNT is set, overflow is set, and TOV is cleared all together
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      _overflowTicks = ticks & 0xFFFF0000;

      // Follow correct 16-bit register access rules by setting the high register first
      *_tcnth = highByte;
      *_tcntl = (uint8_t)ticks;

      // Clear overflow flag
      *_tifr = (1 << _tov);
    }
  }
  else
  {
    // 8-bit timer

    // Ensure that TCNT is set, overflow is set, and TOV is cleared all together
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
  // Use the Arduino overflow variable if defined
#if USE_ARDUINO_TIMER0_OVERFLOW
      if (TIMER0 == _timer)
      {
        timer0_overflow_count = ticks >> 8;
      }
      else
      {
      _overflowTicks = ticks & 0xFFFFFF00;
      }
#elif
      _overflowTicks = ticks & 0xFFFFFF00;
#endif

      *_tcntl = (uint8_t)ticks;

      // Clear overflow flag
      *_tifr = (1 << _tov);
    }
  }
}

ticksExtraRange_t ExtTimer::extend(ticks16_t ticks) const
{
  ticksExtraRange_t overflowTicks;
  uint8_t tifrVal;
  ticks16_t sysTicks;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    overflowTicks = getOverflowTicksInternal();
    tifrVal = *_tifr;
    sysTicks = getSysRange();
  }

  ticksExtraRange_t extTicks = compensateForUnprocessedOverflow(ticks + overflowTicks, tifrVal);

  // Add another overflow if the ticks is before tcnt.
  // We're assuming that ticks is in the future, so so tcnt needs to roll over to get there.
  if (ticks < sysTicks)
  {
    return incrementOverflow(extTicks);
  }
  else
  {
    return extTicks;
  }
}

ticksExtraRange_t ExtTimer::extendTimeInPast(ticks16_t ticks) const
{
  ticksExtraRange_t overflowTicks;
  uint8_t tifrVal;
  ticks16_t sysTicks;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    overflowTicks = getOverflowTicksInternal();
    tifrVal = *_tifr;
    sysTicks = getSysRange();
  }
  
  ticksExtraRange_t extTicks = compensateForUnprocessedOverflow(ticks + overflowTicks, tifrVal);

  // Subtract another overflow if the ticks is after tcnt.
  // We're assuming that ticks is in the past, so so tcnt would have rolled over to get here.
  if (sysTicks <= ticks)
  {
    return decrementOverflow(extTicks);
  }
  else
  {
    return extTicks;
  }
}

ticks16_t ExtTimer::getSysRange() const
{
  if (_tcnth)
  {
    // 16-bit counter
    ticks16_t low, high;

    // Ensure there's no race with someone trying to use the temp register
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      // Follow correct 16-bit register access rules by loading the low register first
      low = *_tcntl;
      high = *_tcnth;
    }

    return low + (high << 8);
  }
  else
  {
    // 8-bit counter
    return *_tcntl;
  }
}

uint16_t ExtTimer::getMaxSysTicks() const {
  if (_tcnth) {
    return UINT16_MAX;
  } else {
    return UINT8_MAX;
  }
}

uint32_t ExtTimer::getOverflowCount() const
{
  ticksExtraRange_t tmp;

  // Use the Arduino overflow variable if defined
#if USE_ARDUINO_TIMER0_OVERFLOW
  if (TIMER0 == _timer)
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      tmp = timer0_overflow_count;
    }

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
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Use the Arduino overflow variable if defined
#ifdef USE_ARDUINO_TIMER0_OVERFLOW
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
  }
}

int ExtTimer::getTimer() const
{
  return _timer;
}
volatile uint8_t *ExtTimer::getTIMSK() const
{
  return _timsk;
}

volatile uint8_t *ExtTimer::getTIFR() const
{
  return _tifr;
}

void ExtTimer::processOverflow()
{
  _overflowTicks = incrementOverflow(_overflowTicks);

  if (_overflowCallback)
  {
    _overflowCallback();
  }
}

void ExtTimer::setOverflowCallback(OverflowCallback cb)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    _overflowCallback = cb;
  }
}

bool ExtTimer::hasUnprocessedOverflow(uint8_t tifrVal) const
{
  return tifrVal & (1 << _tov);
}

ticksExtraRange_t ExtTimer::compensateForUnprocessedOverflow(ticksExtraRange_t ticks, uint8_t tifrVal) const
{
  // See if there was an unprocessed overflow
  if (hasUnprocessedOverflow(tifrVal))
  {
    return incrementOverflow(ticks);
  }
  else
  {
    return ticks;
  }
}

ticksExtraRange_t ExtTimer::getOverflowTicksInternal() const
{
  ticksExtraRange_t tmp;

  // Use the Arduino overflow variable if defined
#ifdef USE_ARDUINO_TIMER0_OVERFLOW
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

  return tmp;
}

ticksExtraRange_t ExtTimer::incrementOverflow(ticksExtraRange_t ticks) const
{
  if (_tcnth)
  {
    // add 65536 to the overflow ticks counter on overflow for 16-bit timers
    return ticks + (1UL << 16);
  }
  else
  {
    // add 256 to the overflow ticks counter on overflow for 8-bit timers
    return ticks + (1UL << 8);
  }
}

ticksExtraRange_t ExtTimer::decrementOverflow(ticksExtraRange_t ticks) const
{
  if (_tcnth)
  {
    // subtract 65536 from the overflow ticks counter on overflow for 16-bit timers
    return ticks - (1UL << 16);
  }
  else
  {
    // subtract 256 from the overflow ticks counter on overflow for 8-bit timers
    return ticks - (1UL << 8);
  }
}

ticksExtraRange_t ExtTimer::getOverflowTicks() const
{
  ticksExtraRange_t overflowTicks;
  uint8_t tifrVal;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    overflowTicks = getOverflowTicksInternal();
    tifrVal = *_tifr;
  }

  return compensateForUnprocessedOverflow(overflowTicks, tifrVal);
}
