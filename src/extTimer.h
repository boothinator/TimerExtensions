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
#include <avr/io.h>

// Extend the range of 16-bit AVR timers
class ExtTimer
{
public:
  ExtTimer(volatile uint8_t *tcntl, volatile uint8_t *tcnth, volatile uint8_t *timsk, uint8_t toie, uint8_t timer);
  
  ticksExtraRange_t get() const;

  // Extend the range of the passed-in ticks
  // Assumes that ticks is in the future
  ticksExtraRange_t extend(ticks16_t ticks) const;

  ticksExtraRange_t extendTimeInPast(ticks16_t ticks) const;

  ticks16_t getSysRange() const;

  uint32_t getOverflowCount() const;
  void resetOverflowCount();

  int getTimer();

  void processOverflow();

private:
  volatile ticksExtraRange_t _overflowTicks = 0;

  volatile uint8_t *_tcntl;
  volatile uint8_t *_tcnth;

  volatile uint8_t *_timsk;
  uint8_t _toie;

  int _timer;
};

#ifdef TCNT0
extern ExtTimer ExtTimer0;
#define HAVE_TCNT0
#endif // TCNT0

#ifdef TCNT1
extern ExtTimer ExtTimer1;
#define HAVE_TCNT1
#endif // TCNT1

#ifdef TCNT2
extern ExtTimer ExtTimer2;
#define HAVE_TCNT2
#endif // TCNT2;

#ifdef TCNT3
extern ExtTimer ExtTimer3;
#define HAVE_TCNT3
#endif // TCNT3

#ifdef TCNT4
extern ExtTimer ExtTimer4;
#define HAVE_TCNT4
#endif // TCNT4

#ifdef TCNT5
extern ExtTimer ExtTimer5;
#define HAVE_TCNT5
#endif // TCNT5


#ifdef ARDUINO_AVR_UNO

#define ExtTimerPin6 ExtTimer0
#define ExtTimerPin5 ExtTimer0

#define ExtTimerPin8 ExtTimer1
#define ExtTimerPin9 ExtTimer1
#define ExtTimerPin10 ExtTimer1

#define ExtTimerPin11 ExtTimer2
#define ExtTimerPin3 ExtTimer2

#endif // ARDUINO_AVR_UNO



#ifdef ARDUINO_AVR_MEGA2560

#define ExtTimerPin13Timer0 ExtTimer0
#define ExtTimerPin4 ExtTimer0

#define ExtTimerPin11 ExtTimer1
#define ExtTimerPin12 ExtTimer1
#define ExtTimerPin13Timer1 ExtTimer1

#define ExtTimerPin10 ExtTimer2
#define ExtTimerPin9 ExtTimer2

#define ExtTimerPin5 ExtTimer3
#define ExtTimerPin2 ExtTimer3
#define ExtTimerPin3 ExtTimer3

#define ExtTimerPin6 ExtTimer4
#define ExtTimerPin7 ExtTimer4
#define ExtTimerPin8 ExtTimer4
#define ExtTimerPin49 ExtTimer4

#define ExtTimerPin46 ExtTimer5
#define ExtTimerPin45 ExtTimer5
#define ExtTimerPin44 ExtTimer5
#define ExtTimerPin48 ExtTimer5

#endif // ARDUINO_AVR_MEGA2560

#endif // TIMER_EXT_EXT_TIMER_H_