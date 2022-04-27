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

#ifndef TIMER_EXT_PULSE_GEN4A_H_
#define TIMER_EXT_PULSE_GEN4A_H_

#include "pulseGen.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "extTimer.h"

#if defined(OCR4A) && defined(OCR4AL)

PulseGen PulseGen4A(&OCR4AL, &OCR4AH, &TCCR4A, &TCCR4B, &TCCR4C, &TIMSK4, COM4A1, COM4A0, FOC4A, OCIE4A, &ExtTimer4);

ISR(TIMER4_COMPA_vect)
{
  PulseGen4A.processCompareEvent();
}

#endif

#endif // TIMER_EXT_PULSE_GEN4A_H_
