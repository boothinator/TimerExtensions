// Extended Range AVR Timer
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

#ifndef TIMER_EXT_EXT_TIMER2_H_
#define TIMER_EXT_EXT_TIMER2_H_

#include "extTimer.h"

#ifdef HAVE_TCNT2

ExtTimer ExtTimer2(&TCNT2, nullptr);

ISR(TIMER2_OVF_vect)
{
  ExtTimer2.processOverflow();
}

#endif // HAVE_TCNT2

#endif // TIMER_EXT_EXT_TIMER2_H_