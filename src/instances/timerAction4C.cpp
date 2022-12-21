// Precise AVR pulse generation
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

#ifndef TIMER_EXT_TIMER_ACTION4C_H_
#define TIMER_EXT_TIMER_ACTION4C_H_

#include "timerAction.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "extTimer.h"

#ifdef OCR4C

TimerAction TimerAction4C(TIMER4C, &ExtTimer4, OCIE4C, OCF4C);

ISR(TIMER4_COMPC_vect)
{
  TimerAction4C.processInterrupt();
}

#endif

#endif // TIMER_EXT_TIMER_ACTION4C_H_
