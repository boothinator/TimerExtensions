// Interrupts
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

// Inspired by Winterrupts.c in the Wiring project
// - http://wiring.uniandes.edu.co

#include "timerInterrupts.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static void icpIntDoNothing(ticks16_t ticks) {
}

static volatile icpIntFuncPtr icpIntFunc[] = {
  icpIntDoNothing,
#if defined(ICR3) || defined(ICR4) || defined(ICR5)
  icpIntDoNothing,
  icpIntDoNothing,
  icpIntDoNothing
#endif
};

int getIcpIntIndex(uint8_t timer)
{
  switch (timer)
  {
    case TIMER1:
    case TIMER1B:
    case TIMER1C:
      return 0;
    case TIMER3:
    case TIMER3B:
    case TIMER3C:
      return 1;
    case TIMER4:
    case TIMER4B:
    case TIMER4C:
      return 2;
    case TIMER5:
    case TIMER5B:
    case TIMER5C:
      return 3;
    default:
      return -1;
  }
}

void attachInputCaptureInterrupt(uint8_t timer, icpIntFuncPtr func, uint8_t edge)
{
  int index = getIcpIntIndex(timer);

  if (index < 0)
  {
    return;
  }

  icpIntFunc[index] = func;

  switch (timer)
  {
#ifdef ICR1
    case TIMER1:
      TIMSK1 |= _BV(ICIE1);
      if (edge == RISING)
        TCCR1B |= _BV(ICES1);
      else
        TCCR1B &= ~_BV(ICES1);
      break;
#endif
#ifdef ICR3
    case TIMER3:
      TIMSK3 |= _BV(ICIE3);
      if (edge == RISING)
        TCCR3B |= _BV(ICES3);
      else
        TCCR3B &= ~_BV(ICES3);
      break;
#endif
#ifdef ICR4
    case TIMER4:
      TIMSK4 |= _BV(ICIE4);
      if (edge == RISING)
        TCCR4B |= _BV(ICES4);
      else
        TCCR4B &= ~_BV(ICES4);
      break;
#endif
#ifdef ICR5
    case TIMER5:
      TIMSK5 |= _BV(ICIE5);
      if (edge == RISING)
        TCCR5B |= _BV(ICES5);
      else
        TCCR5B &= ~_BV(ICES5);
      break;
#endif
    default:
      break;
  }
}

void detachInputCaptureInterrupt(uint8_t timer)
{
  int index = getIcpIntIndex(timer);

  if (index < 0)
  {
    return;
  }

  switch (timer)
  {
#ifdef ICR1
    case TIMER1:
      TIMSK1 &= ~_BV(ICIE1);
      break;
#endif
#ifdef ICR3
    case TIMER3:
      TIMSK3 &= ~_BV(ICIE3);
      break;
#endif
#ifdef ICR4
    case TIMER4:
      TIMSK4 &= ~_BV(ICIE4);
      break;
#endif
#ifdef ICR5
    case TIMER5:
      TIMSK5 &= ~_BV(ICIE5);
      break;
#endif
    default:
      break;
  }

  icpIntFunc[index] = icpIntDoNothing;
}

#define IMPLEMENT_ISR(vect, interrupt, reg) \
  ISR(vect) { \
    uint16_t ticks = reg; \
    icpIntFunc[interrupt](ticks); \
  }

#ifdef ICR1
IMPLEMENT_ISR(TIMER1_CAPT_vect, 0, ICR1);
#endif
#ifdef ICR3
IMPLEMENT_ISR(TIMER3_CAPT_vect, 1, ICR3);
#endif
#ifdef ICR4
IMPLEMENT_ISR(TIMER4_CAPT_vect, 2, ICR4);
#endif
#ifdef ICR5
IMPLEMENT_ISR(TIMER5_CAPT_vect, 3, ICR5);
#endif