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

#ifndef TIMER_EXT_PULSE_GEN4C_H_
#define TIMER_EXT_PULSE_GEN4C_H_

#include "pulseGen.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "extTimer.h"

#if defined(OCR4C) && defined(OCR4CL)

PulseGen PulseGen4C(&OCR4CL, &OCR4CH, &TCCR4A, &TCCR4B, &TCCR4C, &TIMSK4, COM4C1, COM4C0, FOC4C, OCIE4C, &ExtTimer4);

ISR(TIMER4_COMPC_vect)
{
  PulseGen4C.processCompareEvent();
}

#endif

#endif // TIMER_EXT_PULSE_GEN4C_H_
