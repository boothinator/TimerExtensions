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

#include <avr/interrupt.h>
#include <assert.h>
#include <stdint.h>

namespace {

// TODO: Implement 8-bit timers

volatile uint8_t *getTimerTCCRA(uint8_t timer)
{
  switch (timer)
  {
    case TIMER0:
    case TIMER0B:
      return &TCCR0A;
    case TIMER1:
    case TIMER1B:
    case TIMER1C:
      return &TCCR1A;
    case TIMER2:
    case TIMER2A:
    case TIMER2B:
      return &TCCR2A;
#ifdef TCCR3A
    case TIMER3:
    case TIMER3B:
    case TIMER3C:
      return &TCCR3A;
#endif
#ifdef TCCR4A
    case TIMER4:
    case TIMER4B:
    case TIMER4C:
      return &TCCR4A;
#endif
#ifdef TCCR5A
    case TIMER5:
    case TIMER5B:
    case TIMER5C:
      return &TCCR5A;
#endif
    default:
      return nullptr;
  }
}

volatile uint8_t *getTimerTCCRB(uint8_t timer)
{
  switch (timer)
  {
    case TIMER0:
      return &TCCR0B;
    case TIMER1:
      return &TCCR1B;
    case TIMER2:
      return &TCCR2B;
#ifdef TCCR3A
    case TIMER3:
      return &TCCR3B;
#endif
#ifdef TCCR4A
    case TIMER4:
      return &TCCR4B;
#endif
#ifdef TCCR5A
    case TIMER5:
      return &TCCR5B;
#endif
    default:
      return nullptr;
  }
}

volatile uint8_t *getTimerTIFR(uint8_t timer)
{
  switch (timer)
  {
    case TIMER0:
      return &TIFR0;
    case TIMER1:
      return &TIFR1;
    case TIMER2:
      return &TIFR2;
#ifdef TIFR3A
    case TIMER3:
      return &TIFR3;
#endif
#ifdef TIFR4A
    case TIMER4:
      return &TIFR4;
#endif
#ifdef TIFR5A
    case TIMER5:
      return &TIFR5;
#endif
    default:
      return nullptr;
  }
}

} // namespace

bool configureTimerClock(uint8_t timer, TimerClock clock)
{
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  uint8_t cs;

  if (TimerClock::None == clock)
  {
    cs = 0;
  }
  else if (TimerClock::Clk == clock)
  {
    cs = 0b00000001;
  }
  else if (TimerClock::ClkDiv8 == clock)
  {
    cs = 0b00000010;
  }
  else if (TimerClock::ClkDiv32 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000011;
    }
    else
    {
      return false;
    }
  }
  else if (TimerClock::ClkDiv64 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000100;
    }
    else
    {
      cs = 0b00000011;
    }
  }
  else if (TimerClock::ClkDiv128 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000011;
    }
    else
    {
      return false;
    }
  }
  else if (TimerClock::ClkDiv256 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000110;
    }
    else
    {
      cs = 0b00000100;
    }
  }
  else if (TimerClock::ClkDiv1024 == clock)
  {
    if (TIMER2 == timer)
    {
      cs = 0b00000111;
    }
    else
    {
      cs = 0b00000101;
    }
  }

  *TCCRB = (*TCCRB & 0b11111000) | cs;
  return true;
}

bool configureTimerMode(uint8_t timer, TimerMode mode)
{
  volatile uint8_t *TCCRA = getTimerTCCRA(timer);
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  switch (mode)
  {
    case TimerMode::Normal:
      *TCCRA = (*TCCRA & 0b11111100);
      *TCCRB = (*TCCRB & 0b11000111);
      return true;
  }

  return false;
}

uint8_t inputCapturePinToTimer(uint8_t pin)
{
  switch (pin)
  {
#ifdef ARDUINO_AVR_UNO
    case 8:
      return TIMER1;
#endif
#ifdef ARDUINO_AVR_MEGA2560
    case 5:
      return TIMER3;
    case 48:
      return TIMER5;
    case 49:
      return TIMER4;
#endif
    default:
      return NOT_ON_TIMER;
  }
}

constexpr uint8_t ICNC = 7;

void setInputCaptureNoiseCancellerEnabled(uint8_t timer, bool enabled)
{
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  if (enabled)
  {
    *ptccrb |= _BV(ICNC);
  }
  else
  {
    *ptccrb &= ~_BV(ICNC);
  }
}

uint8_t getInputCaptureNoiseCancellerEnabled(uint8_t timer)
{
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  return (*ptccrb & _BV(ICNC)) == _BV(ICNC);
}


constexpr uint8_t ICF = 5;

bool hasInputCapture(uint8_t timer)
{
  volatile uint8_t *ptifr = getTimerTIFR(timer);

  return (*ptifr & _BV(ICF)) == _BV(ICF);
}

void clearInputCapture(uint8_t timer)
{
  volatile uint8_t *ptifr = getTimerTIFR(timer);

  *ptifr |= _BV(ICF);
}


int clockCyclesPerTick(TimerClock clock)
{
  switch (clock)
  {
    case TimerClock::Clk:
      return 1;
    case TimerClock::ClkDiv8:
      return 8;
    case TimerClock::ClkDiv32:
      return 32;
    case TimerClock::ClkDiv64:
      return 64;
    case TimerClock::ClkDiv128:
      return 128;
    case TimerClock::ClkDiv256:
      return 256;
    case TimerClock::ClkDiv1024:
      return 1024;
    default:
      return 0;
  }
}

constexpr int clockCyclesPerMillisecond = F_CPU / 1000;
constexpr int clockCyclesPerMicrosecond = F_CPU / 1000000;

int ticksToClockCycles(ticksExtraRange_t ticks, TimerClock clock)
{
  return ticks * clockCyclesPerTick(clock);
}

int ticksToMilliseconds(ticksExtraRange_t ticks, TimerClock clock)
{
  return ticks * clockCyclesPerTick(clock) / clockCyclesPerMillisecond;
}

int ticksToMicroseconds(ticksExtraRange_t ticks, TimerClock clock)
{
  return ticks * clockCyclesPerTick(clock) / clockCyclesPerMicrosecond;
}

ticksExtraRange_t clockCyclesToTicks(uint32_t clockCycles, TimerClock clock)
{
  return clockCycles / clockCyclesPerTick(clock);
}

ticksExtraRange_t millisecondsToTicks(uint32_t milliseconds, TimerClock clock)
{
  return milliseconds * clockCyclesPerMillisecond / clockCyclesPerTick(clock);
}

ticksExtraRange_t microsecondsToTicks(uint32_t microseconds, TimerClock clock)
{
  return microseconds * clockCyclesPerMicrosecond / clockCyclesPerTick(clock);
}




TimerConfig getTimerConfig(uint8_t timer)
{
  return {
    *getTimerTCCRA(timer),
    *getTimerTCCRB(timer)
  };
}

void restoreTimerConfig(uint8_t timer, TimerConfig config)
{

  volatile uint8_t *ptccra = getTimerTCCRA(timer);
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  uint8_t tccraVal = (*ptccra & 0b11111100) | (config.tccra & 0b00000011);
  uint8_t tccrbVal = config.tccrb;

  *ptccra = tccraVal;
  *ptccrb = tccrbVal;
}