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

#include "timerUtil.h"

#include <avr/interrupt.h>
#include <assert.h>
#include <stdint.h>

namespace {

// TODO: Implement 8-bit timers

volatile uint8_t *getTimerTCCRA(uint8_t timer)
{
  switch (timer)
  {
    case TIMER0:
    case TIMER0B:
      return &TCCR0A;
    case TIMER1:
    case TIMER1B:
    case TIMER1C:
      return &TCCR1A;
    case TIMER2:
    case TIMER2A:
    case TIMER2B:
      return &TCCR2A;
#ifdef TCCR3A
    case TIMER3:
    case TIMER3B:
    case TIMER3C:
      return &TCCR3A;
#endif
#ifdef TCCR4A
    case TIMER4:
    case TIMER4B:
    case TIMER4C:
      return &TCCR4A;
#endif
#ifdef TCCR5A
    case TIMER5:
    case TIMER5B:
    case TIMER5C:
      return &TCCR5A;
#endif
    default:
      return nullptr;
  }
}

volatile uint8_t *getTimerTCCRB(uint8_t timer)
{
  switch (timer)
  {
    case TIMER0:
      return &TCCR0B;
    case TIMER1:
      return &TCCR1B;
    case TIMER2:
      return &TCCR2B;
#ifdef TCCR3A
    case TIMER3:
      return &TCCR3B;
#endif
#ifdef TCCR4A
    case TIMER4:
      return &TCCR4B;
#endif
#ifdef TCCR5A
    case TIMER5:
      return &TCCR5B;
#endif
    default:
      return nullptr;
  }
}

} // namespace

bool configureTimerClock(uint8_t timer, TimerClock clock)
{
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  uint8_t cs;

  if (TimerClock::None == clock)
  {
    cs = 0;
  }
  else if (TimerClock::Clk == clock)
  {
    cs = 0b00000001;
  }
  else if (TimerClock::ClkDiv8 == clock)
  {
    cs = 0b00000010;
  }
  else if (TimerClock::ClkDiv32 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000011;
    }
    else
    {
      return false;
    }
  }
  else if (TimerClock::ClkDiv64 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000100;
    }
    else
    {
      cs = 0b00000011;
    }
  }
  else if (TimerClock::ClkDiv128 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000011;
    }
    else
    {
      return false;
    }
  }
  else if (TimerClock::ClkDiv256 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000110;
    }
    else
    {
      cs = 0b00000100;
    }
  }
  else if (TimerClock::ClkDiv1024 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000111;
    }
    else
    {
      cs = 0b00000101;
    }
  }

  *TCCRB = (*TCCRB & 0b11111000) | cs;
  return true;
}

bool configureTimerMode(uint8_t timer, TimerMode mode)
{
  volatile uint8_t *TCCRA = getTimerTCCRA(timer);
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  switch (mode)
  {
    case TimerMode::Normal:
      *TCCRA = (*TCCRA & 0b11111100);
      *TCCRB = (*TCCRB & 0b11000111);
      return true;
  }

  return false;
}


TimerConfig getTimerConfig(uint8_t timer)
{
  return {
    *getTimerTCCRA(timer),
    *getTimerTCCRB(timer)
  };
}

void restoreTimerConfig(uint8_t timer, TimerConfig config)
{

  volatile uint8_t *ptccra = getTimerTCCRA(timer);
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  uint8_t tccraVal = (*ptccra & 0b11111100) | (config.tccra & 0b00000011);
  uint8_t tccrbVal = config.tccrb;

  *ptccra = tccraVal;
  *ptccrb = tccrbVal;
}