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

#ifndef TIMER_EXT_PULSE_GEN5C_H_
#define TIMER_EXT_PULSE_GEN5C_H_

#include "pulseGen.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "extTimer.h"

#ifdef OCR5C

PulseGen PulseGen5C(&OCR5CL, &OCR5CH, &TCCR5A, &TCCR5B, &TCCR5C, &TIMSK5, COM5C1, COM5C0, FOC5C, OCIE5C, &ExtTimer5);

ISR(TIMER5_COMPC_vect)
{
  PulseGen5C.processCompareEvent();
}

#endif

#endif // TIMER_EXT_PULSE_GEN5C_H_
