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

typedef void (*icpIntFuncPtr)(ticksExtraRange_t ticks);

void attachInputCaptureInterrupt(Timer timer, icpIntFuncPtr func, Edge edge);

void detachInputCaptureInterrupt(Timer timer);

#endif // TIMER_EXT_TIMER_INTERRUPTS_H_