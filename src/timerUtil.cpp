// AVR timer utils
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

#include "timerUtil.h"

namespace {

volatile uint8_t *getTimerTCCRA(Timer timer)
{
  switch (timer)
  {
    case Timer::Timer1:
      return &TCCR1A;
    case Timer::Timer3:
      return &TCCR3A;
    case Timer::Timer4:
      return &TCCR4A;
    case Timer::Timer5:
      return &TCCR5A;
    default:
      return nullptr;
  }
}

volatile uint8_t *getTimerTCCRB(Timer timer)
{
  switch (timer)
  {
    case Timer::Timer1:
      return &TCCR1B;
    case Timer::Timer3:
      return &TCCR3B;
    case Timer::Timer4:
      return &TCCR4B;
    case Timer::Timer5:
      return &TCCR5B;
    default:
      return nullptr;
  }
}

} // namespace

void configureTimerClock(Timer timer, TimerClock clock)
{
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  switch (clock)
  {
    case TimerClock::None:
      *TCCRB = *TCCRB & 0b11111000;
      break;
    case TimerClock::Clk:
      *TCCRB = (*TCCRB & 0b11111000) | 0b00000001;
      break;
    case TimerClock::ClkDiv8:
      *TCCRB = (*TCCRB & 0b11111000) | 0b00000010;
      break;
  }
}

void configureTimerMode(Timer timer, TimerMode mode)
{
  volatile uint8_t *TCCRA = getTimerTCCRA(timer);
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  switch (mode)
  {
    case TimerMode::Normal:
      *TCCRA = (*TCCRA & 0b11111100);
      *TCCRB = (*TCCRB & 0b11000111);
  }
}
