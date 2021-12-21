// AVR Timer Types
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

#ifndef TIMER_EXT_TIMER_TYPES_H_
#define TIMER_EXT_TIMER_TYPES_H_

#include <stdint.h>

enum class Timer {Timer0, Timer1, Timer2, Timer3, Timer4, Timer5};

typedef uint8_t  ticks8_t;
typedef uint16_t ticks16_t;
typedef uint32_t ticksExtraRange_t;

#endif // TIMER_EXT_TIMER_TYPES_H_