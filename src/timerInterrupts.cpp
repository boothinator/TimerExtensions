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

int getIcpIntIndex(Timer timer)
{
  switch (timer)
  {
    case Timer::Timer1:
      return 0;
    case Timer::Timer3:
      return 1;
    case Timer::Timer4:
      return 2;
    case Timer::Timer5:
      return 3;
    default:
      return -1;
  }
}

void attachInputCaptureInterrupt(Timer timer, icpIntFuncPtr func, Edge edge)
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
    case Timer::Timer1:
      TIMSK1 |= _BV(ICIE1);
      break;
#endif
#ifdef ICR3
    case Timer::Timer3:
      TIMSK3 |= _BV(ICIE3);
      break;
#endif
#ifdef ICR4
    case Timer::Timer4:
      TIMSK4 |= _BV(ICIE4);
      break;
#endif
#ifdef ICR5
    case Timer::Timer5:
      TIMSK5 |= _BV(ICIE5);
      break;
#endif
    default:
      break;
  }
}

void detachInputCaptureInterrupt(Timer timer)
{
  int index = getIcpIntIndex(timer);

  if (index < 0)
  {
    return;
  }

  switch (timer)
  {
#ifdef ICR1
    case Timer::Timer1:
      TIMSK1 &= ~_BV(ICIE1);
      break;
#endif
#ifdef ICR3
    case Timer::Timer3:
      TIMSK3 &= ~_BV(ICIE3);
      break;
#endif
#ifdef ICR4
    case Timer::Timer4:
      TIMSK4 &= ~_BV(ICIE4);
      break;
#endif
#ifdef ICR5
    case Timer::Timer5:
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