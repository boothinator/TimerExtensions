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

#ifndef TIMER_EXT_PULSE_GEN3A_H_
#define TIMER_EXT_PULSE_GEN3A_H_

#include "pulseGen.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "extTimer.h"

PulseGen PulseGen3A(&OCR3AL, &OCR3AH, &TCCR3A, &TCCR3B, &TCCR3C, &TIMSK3, COM3A1, COM3A0, FOC3A, OCIE3A, &ExtTimer3);

ISR(TIMER3_COMPA_vect)
{
  PulseGen3A.processCompareEvent();
}

#endif // TIMER_EXT_PULSE_GEN3A_H_
