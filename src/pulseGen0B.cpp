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

#ifndef TIMER_EXT_PULSE_GEN0B_H_
#define TIMER_EXT_PULSE_GEN0B_H_

#include "pulseGen.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "extTimer.h"

// Note: The Force Output Compare bits are on TCCRxB for 8-bit counters, but
// they're on TCCRxC for 16-bit counters
PulseGen PulseGen0B(&OCR0B, nullptr, &TCCR0A, &TCCR0B, &TCCR0B, COM0B1, COM0B0, FOC0B, &ExtTimer0);

ISR(TIMER0_COMPB_vect)
{
  PulseGen0B.processCompareEvent();
}

#endif // TIMER_EXT_PULSE_GEN0B_H_
