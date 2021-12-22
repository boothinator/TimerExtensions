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

#ifndef TIMER_EXT_PULSE_GEN3C_H_
#define TIMER_EXT_PULSE_GEN3C_H_

#include "pulseGen.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "extTimer.h"

#ifdef OCR3C

PulseGen PulseGen3C(&OCR3CL, &OCR3CH, &TCCR3A, &TCCR3B, &TCCR3C, &TIMSK3, COM3C1, COM3C0, FOC3C, OCIE3C, &ExtTimer3);

ISR(TIMER3_COMPC_vect)
{
  PulseGen3C.processCompareEvent();
}

#endif

#endif // TIMER_EXT_PULSE_GEN3C_H_
