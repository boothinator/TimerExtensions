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

#ifndef NOT_ON_TIMER

#define NOT_ON_TIMER 0
#define TIMER0A 1
#define TIMER0B 2
#define TIMER1A 3
#define TIMER1B 4
#define TIMER1C 5
#define TIMER2  6
#define TIMER2A 7
#define TIMER2B 8

#define TIMER3A 9
#define TIMER3B 10
#define TIMER3C 11
#define TIMER4A 12
#define TIMER4B 13
#define TIMER4C 14
#define TIMER4D 15
#define TIMER5A 16
#define TIMER5B 17
#define TIMER5C 18

#endif

#define TIMER0 TIMER0A
#define TIMER1 TIMER1A
#define TIMER3 TIMER3A
#define TIMER4 TIMER4A
#define TIMER5 TIMER5A

#ifndef CHANGE

#define CHANGE 1
#define FALLING 2
#define RISING 3

#endif

typedef uint8_t  ticks8_t;
typedef uint16_t ticks16_t;
typedef uint32_t ticksExtraRange_t;

#endif // TIMER_EXT_TIMER_TYPES_H_