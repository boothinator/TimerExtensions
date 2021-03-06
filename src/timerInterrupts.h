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

#ifndef TIMER_EXT_TIMER_INTERRUPTS_H_
#define TIMER_EXT_TIMER_INTERRUPTS_H_

#include "timerTypes.h"

typedef void (*icpIntFuncPtr)(ticks16_t ticks);

void attachInputCaptureInterrupt(uint8_t timer, icpIntFuncPtr func, uint8_t edge);

void detachInputCaptureInterrupt(uint8_t timer);

#endif // TIMER_EXT_TIMER_INTERRUPTS_H_