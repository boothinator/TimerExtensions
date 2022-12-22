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

volatile uint8_t *getTimerTCCRA(uint8_t timer)
{
  switch(timer)
  {
    // XXX fix needed for atmega8
    #if defined(TCCR0) && !defined(__AVR_ATmega8__)
    case TIMER0:
    case TIMER0A:
    case TIMER0B:
      return &TCCR0;
    #endif

    #if defined(TCCR0A)
    //case TIMER0:
    case TIMER0A:
    case TIMER0B:
      return &TCCR0A;
    #endif

    #if defined(TCCR1A)
    //case TIMER1:
    case TIMER1A:
    case TIMER1B:
    case TIMER1C:
      return &TCCR1A;
    #endif

    #if defined(TCCR2)
    case TIMER2:
    case TIMER2A:
    case TIMER2B:
      return &TCCR2;
    #endif

    #if defined(TCCR2A)
    case TIMER2:
    case TIMER2A:
    case TIMER2B:
      return &TCCR2A;
    #endif

    #if defined(TCCR3A)
    //case TIMER3:
    case TIMER3A:
    case TIMER3B:
    case TIMER3C:
      return &TCCR3A;
    #endif

    #if defined(TCCR4A)
    //case TIMER4:
    case TIMER4A:
    case TIMER4B:
    case TIMER4C:
      return &TCCR4A;
    #endif

    #if defined(TCCR5A)
    //case TIMER5:
    case TIMER5A:
    case TIMER5B:
    case TIMER5C:
      return &TCCR5A;
    #endif

    case NOT_ON_TIMER:
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
#ifdef TCCR2A
    case TIMER2:
      return &TCCR2B;
#endif
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
#ifdef TIFR2A
    case TIMER2:
      return &TIFR2;
#endif
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

TimerType getTimerType(uint8_t timer)
{
  switch (timer)
  {
    case TIMER0:
      return TimerType::_8Bit;
    case TIMER1:
      return TimerType::_16Bit;
    case TIMER2:
      return TimerType::_8Bit;
#ifdef TIFR3
    case TIMER3:
      return TimerType::_16Bit;
#endif
#ifdef TIFR4
    case TIMER4:
      return TimerType::_16Bit;
#endif
#ifdef TIFR5
    case TIMER5:
      return TimerType::_16Bit;
#endif
    default:
      return TimerType::NotATimer;
  }
}

static const uint8_t InvalidClockSelect = UINT8_MAX;

uint8_t getClockSelectBits(uint8_t timer, TimerClock clock)
{
  switch (clock)
  {
    case TimerClock::None:
      return 0;
    case TimerClock::Clk:
      return 0b00000001;
    case TimerClock::ClkDiv8:
      return 0b00000010;
    default:
      // Fall through
      break;
  }

  if (TIMER2 == timer)
  {
    switch (clock)
    {
      case TimerClock::ClkDiv32:
        return 0b00000011;
      case TimerClock::ClkDiv64:
        return 0b00000100;
      case TimerClock::ClkDiv128:
        return 0b00000101;
      case TimerClock::ClkDiv256:
        return 0b00000110;
      case TimerClock::ClkDiv1024:
        return 0b00000111;
      default:
        // Fall through
        break;
    }
  }
  else
  {
    switch (clock)
    {
      case TimerClock::ClkDiv64:
        return 0b00000011;
      case TimerClock::ClkDiv256:
        return 0b00000100;
      case TimerClock::ClkDiv1024:
        return 0b00000101;
      default:
        // Fall through
        break;
    }
  }

  return InvalidClockSelect;
}

bool setTimerClock(uint8_t timer, TimerClock clock)
{
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  if (!TCCRB)
  {
    return false;
  }

  uint8_t cs = getClockSelectBits(timer, clock);

  if (InvalidClockSelect == cs)
  {
    return false;
  }

  *TCCRB = (*TCCRB & 0b11111000) | cs;
  return true;
}

TimerClock getTimerClock(uint8_t timer)
{
  volatile uint8_t *TCCRB = getTimerTCCRB(timer);

  if (!TCCRB)
  {
    return TimerClock::None;
  }

  uint8_t csBits = *TCCRB & 0b00000111;

  switch (csBits)
  {
    case 0:
      return TimerClock::None;
    case 0b00000001:
      return TimerClock::Clk;
    case 0b00000010:
      return TimerClock::ClkDiv8;
    default:
      // Fall through
      break;
  }

  if (TIMER2 == timer)
  {
    switch (csBits)
    {
      case 0b00000011:
        return TimerClock::ClkDiv32;
      case 0b00000100:
        return TimerClock::ClkDiv64;
      case 0b00000101:
        return TimerClock::ClkDiv128;
      case 0b00000110:
        return TimerClock::ClkDiv256;
      case 0b00000111:
        return TimerClock::ClkDiv1024;
      default:
        // Fall through
        break;
    }
  }
  else
  {
    switch (csBits)
    {
      case 0b00000011:
        return TimerClock::ClkDiv64;
      case 0b00000100:
        return TimerClock::ClkDiv256;
      case 0b00000101:
        return TimerClock::ClkDiv1024;
      default:
        // Fall through
        break;
    }
  }

  return TimerClock::None;
}

constexpr uint8_t WGM0_TCCRA_8BIT = 0;
constexpr uint8_t WGM1_TCCRA_8BIT = 1;
constexpr uint8_t WGM2_TCCRB_8BIT = 3;

bool configureTimerMode8Bit(uint8_t timer, TimerMode mode, TimerResolution resolution)
{
  volatile uint8_t *ptccra = getTimerTCCRA(timer);
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  if (!ptccra || !ptccrb)
  {
    return false;
  }

  uint8_t normalTccra = *ptccra & 0b11111100;
  uint8_t normalTccrb = *ptccrb & 0b11110111;

  switch (mode)
  {
    case TimerMode::Normal:
      *ptccra = normalTccra;
      *ptccrb = normalTccrb;
      return true;
    case TimerMode::CTC:
      switch (resolution)
      {
        case TimerResolution::OCRA:
          *ptccra = normalTccra;
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_8BIT);
          return true;
        default:
          return false;
      }
    case TimerMode::FastPWM:
      switch (resolution)
      {
        case TimerResolution::_8Bit:
          *ptccra = normalTccra | _BV(WGM0_TCCRA_8BIT);
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_8BIT);
          return true;
        case TimerResolution::OCRA:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_8BIT) | _BV(WGM0_TCCRA_8BIT);
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_8BIT);
          return true;
        default:
          return false;
      }
    case TimerMode::PWM_PC:
      switch (resolution)
      {
        case TimerResolution::_8Bit:
          *ptccra = normalTccra | _BV(WGM0_TCCRA_8BIT);
          *ptccrb = normalTccrb;
          return true;
        case TimerResolution::OCRA:
          *ptccra = normalTccra | _BV(WGM0_TCCRA_8BIT);
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_8BIT);
          return true;
        default:
          return false;
      }
    default:
      return false;
  }
}

constexpr uint8_t WGM0_TCCRA_16BIT = 0;
constexpr uint8_t WGM1_TCCRA_16BIT = 1;
constexpr uint8_t WGM2_TCCRB_16BIT = 3;
constexpr uint8_t WGM3_TCCRB_16BIT = 4;

bool configureTimerMode16Bit(uint8_t timer, TimerMode mode, TimerResolution resolution)
{
  volatile uint8_t *ptccra = getTimerTCCRA(timer);
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  if (!ptccra || !ptccrb)
  {
    return false;
  }

  uint8_t normalTccra = *ptccra & 0b11111100;
  uint8_t normalTccrb = *ptccrb & 0b11100111;

  switch (mode)
  {
    case TimerMode::Normal:
      *ptccra = normalTccra;
      *ptccrb = normalTccrb;
      return true;
    case TimerMode::CTC:
      switch (resolution)
      {
        case TimerResolution::OCRA:
          *ptccra = normalTccra;
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_16BIT);
          return true;
        case TimerResolution::ICR:
          *ptccra = normalTccra;
          *ptccrb = normalTccrb | _BV(WGM3_TCCRB_16BIT) | _BV(WGM2_TCCRB_16BIT);
          return true;
        default:
          return false;
      }
    case TimerMode::FastPWM:
      switch (resolution)
      {
        case TimerResolution::_8Bit:
          *ptccra = normalTccra | _BV(WGM0_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_16BIT);
          return true;
        case TimerResolution::_9Bit:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_16BIT);
          return true;
        case TimerResolution::_10Bit:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT) | _BV(WGM0_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM2_TCCRB_16BIT);
          return true;
        case TimerResolution::ICR:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM3_TCCRB_16BIT) | _BV(WGM2_TCCRB_16BIT);
          return true;
        case TimerResolution::OCRA:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT) | _BV(WGM0_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM3_TCCRB_16BIT) | _BV(WGM2_TCCRB_16BIT);
          return true;
        default:
          return false;
      }
    case TimerMode::PWM_PC:
      switch (resolution)
      {
        case TimerResolution::_8Bit:
          *ptccra = normalTccra | _BV(WGM0_TCCRA_16BIT);
          *ptccrb = normalTccrb;
          return true;
        case TimerResolution::_9Bit:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT);
          *ptccrb = normalTccrb;
          return true;
        case TimerResolution::_10Bit:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT) | _BV(WGM0_TCCRA_16BIT);
          *ptccrb = normalTccrb;
          return true;
        case TimerResolution::ICR:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM3_TCCRB_16BIT);
          return true;
        case TimerResolution::OCRA:
          *ptccra = normalTccra | _BV(WGM1_TCCRA_16BIT) | _BV(WGM0_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM3_TCCRB_16BIT);
          return true;
        default:
          return false;
      }
    case TimerMode::PWM_PFC:
      switch (resolution)
      {
        case TimerResolution::ICR:
          *ptccra = normalTccra;
          *ptccrb = normalTccrb | _BV(WGM3_TCCRB_16BIT);
          return true;
        case TimerResolution::OCRA:
          *ptccra = normalTccra | _BV(WGM0_TCCRA_16BIT);
          *ptccrb = normalTccrb | _BV(WGM3_TCCRB_16BIT);
          return true;
        default:
          return false;
      }
    default:
      return false;
  }
}

bool setTimerMode(uint8_t timer, TimerMode mode, TimerResolution resolution)
{
  TimerType type = getTimerType(timer);

  switch (type)
  {
    case TimerType::_8Bit:
      return configureTimerMode8Bit(timer, mode, resolution);
    case TimerType::_16Bit:
      return configureTimerMode16Bit(timer, mode, resolution);
    default:
      return false;
  }
}

void setOutputCompareAction(int timer, CompareAction action)
{
  volatile uint8_t *tccra = getTimerTCCRA(timer);

  if (tccra)
  {
    uint8_t com = (uint8_t)action;

    switch (timer)
    {
      case TIMER0A:
      case TIMER1A:
      case TIMER2A:
      case TIMER3A:
      case TIMER4A:
      case TIMER5A:
        *tccra = (*tccra & 0b00111111) | (com << 6);
        break;
      case TIMER0B:
      case TIMER1B:
      case TIMER2B:
      case TIMER3B:
      case TIMER4B:
      case TIMER5B:
        *tccra = (*tccra & 0b11001111) | (com << 4);
        break;
      case TIMER1C:
      case TIMER3C:
      case TIMER4C:
      case TIMER5C:
        *tccra = (*tccra & 0b11110011) | (com << 2);
        break;
    }
  }
}

CompareAction getOutputCompareAction(int timer)
{
  volatile uint8_t *tccra = getTimerTCCRA(timer);

  if (tccra)
  {
    switch (timer)
    {
      case TIMER0A:
      case TIMER1A:
      case TIMER2A:
      case TIMER3A:
      case TIMER4A:
      case TIMER5A:
        return (CompareAction) ((*tccra & 0b11000000) >> 6);
      case TIMER0B:
      case TIMER1B:
      case TIMER2B:
      case TIMER3B:
      case TIMER4B:
      case TIMER5B:
        return (CompareAction) ((*tccra & 0b00110000) >> 4);
      case TIMER1C:
      case TIMER3C:
      case TIMER4C:
      case TIMER5C:
        return (CompareAction) ((*tccra & 0b00001100) >> 2);
    }
  }

  return CompareAction::Nothing;
}

void setOutputCompareTicks(int timer, ticks16_t val)
{
  /* Copied from wiring_analog.c and modified */
  switch(timer)
  {
  	#if defined(TCCR0) && defined(COM00) && !defined(__AVR_ATmega8__)
    case TIMER0A:
      OCR0 = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR0A) && defined(COM0A1)
    case TIMER0A:
      OCR0A = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR0A) && defined(COM0B1)
    case TIMER0B:
      OCR0B = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR1A) && defined(COM1A1)
    case TIMER1A:
      OCR1A = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR1A) && defined(COM1B1)
    case TIMER1B:
      OCR1B = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR1A) && defined(COM1C1)
    case TIMER1C:
      OCR1C = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR2) && defined(COM21)
    case TIMER2:
      OCR2 = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR2A) && defined(COM2A1)
    case TIMER2A:
      OCR2A = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR2A) && defined(COM2B1)
    case TIMER2B:
      OCR2B = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR3A) && defined(COM3A1)
    case TIMER3A:
      OCR3A = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR3A) && defined(COM3B1)
    case TIMER3B:
      OCR3B = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR3A) && defined(COM3C1)
    case TIMER3C:
      OCR3C = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR4A)
    case TIMER4A:
      OCR4A = val;	// set pwm duty
      break;
    #endif
    
    #if defined(TCCR4A) && defined(COM4B1)
    case TIMER4B:
      OCR4B = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR4A) && defined(COM4C1)
    case TIMER4C:
      OCR4C = val; // set pwm duty
      break;
    #endif
      
    #if defined(TCCR4C) && defined(COM4D1)
    case TIMER4D:
      OCR4D = val;	// set pwm duty
      break;
    #endif

            
    #if defined(TCCR5A) && defined(COM5A1)
    case TIMER5A:
      OCR5A = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR5A) && defined(COM5B1)
    case TIMER5B:
      OCR5B = val; // set pwm duty
      break;
    #endif

    #if defined(TCCR5A) && defined(COM5C1)
    case TIMER5C:
      OCR5C = val; // set pwm duty
      break;
    #endif

    case NOT_ON_TIMER:
    default:
      break;
  }
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

  if (!ptccrb)
  {
    return;
  }

  if (enabled)
  {
    *ptccrb |= _BV(ICNC);
  }
  else
  {
    *ptccrb &= ~_BV(ICNC);
  }
}

bool getInputCaptureNoiseCancellerEnabled(uint8_t timer)
{
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  if (!ptccrb)
  {
    return false;
  }

  return (*ptccrb & _BV(ICNC)) == _BV(ICNC);
}


constexpr uint8_t ICF = 5;

bool hasInputCapture(uint8_t timer)
{
  volatile uint8_t *ptifr = getTimerTIFR(timer);

  if (!ptifr)
  {
    return false;
  }

  return (*ptifr & _BV(ICF)) == _BV(ICF);
}

void clearInputCapture(uint8_t timer)
{
  volatile uint8_t *ptifr = getTimerTIFR(timer);

  if (!ptifr)
  {
    return;
  }

  *ptifr |= _BV(ICF);
}

constexpr uint8_t ICES = 6;

void setInputCaptureEdge(uint8_t timer, uint8_t edge)
{
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  if (!ptccrb)
  {
    return;
  }

  if (edge == RISING)
  {
    *ptccrb |= _BV(ICES);
  }
  else
  {
    *ptccrb &= ~_BV(ICES);
  }
}

ticks16_t getInputCapture(uint8_t timer, bool clear)
{
  clearInputCapture(timer);

  switch (timer)
  {
    case TIMER1:
      return ICR1;
#ifdef ICR3A
    case TIMER3:
      return ICR3;
#endif
#ifdef ICR4A
    case TIMER4:
      return ICR4;
#endif
#ifdef ICR5A
    case TIMER5:
      return ICR5;
#endif
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


ticks16_t getTimerValue(uint8_t timer)
{
  switch (timer)
  {
    case TIMER0:
      return TCNT0;
    case TIMER1:
      return TCNT1;
#ifdef TCNT2A
    case TIMER2:
      return TCNT2;
#endif
#ifdef TCNT3A
    case TIMER3:
      return TCNT3;
#endif
#ifdef TCNT4A
    case TIMER4:
      return TCNT4;
#endif
#ifdef TCNT5A
    case TIMER5:
      return TCNT5;
#endif
    default:
      return 0;
  }
}

void setTimerValue(uint8_t timer, ticks16_t ticks)
{
  switch (timer)
  {
    case TIMER0:
      TCNT0 = ticks;
      break;
    case TIMER1:
      TCNT1 = ticks;
      break;
#ifdef TCNT2A
    case TIMER2:
      TCNT2 = ticks;
      break;
#endif
#ifdef TCNT3A
    case TIMER3:
      TCNT3 = ticks;
      break;
#endif
#ifdef TCNT4A
    case TIMER4:
      TCNT4 = ticks;
      break;
#endif
#ifdef TCNT5A
    case TIMER5:
      TCNT5 = ticks;
      break;
#endif
    default:
      break;
  }
}


bool setModulatorType(uint8_t pin, ModType mod)
{
  switch (pin)
  {
#if defined(ARDUINO_AVR_MEGA2560) && defined(PORTB7)
    case 13:
      switch (mod)
      {
        case ModType::And:
          PORTB &= ~_BV(PORTB7);
          return true;
        case ModType::Or:
          PORTB |= _BV(PORTB7);
          return true;
      }
#endif
    default:
      return false;
  }
}

void resetSynchronousPrescaler()
{
  GTCCR |= _BV(PSRSYNC);
}

void resetAsynchronousPrescaler()
{
  GTCCR |= _BV(PSRASY);
}

void setTimerSynchronizationModeEnabled(bool enabled)
{
  if (enabled)
  {
    GTCCR |= _BV(TSM);
  }
  else
  {
    GTCCR &= ~_BV(TSM);
  }
}

void stopAllTimersAndSynchronize()
{
  GTCCR |= _BV(TSM) | _BV(PSRASY) | _BV(PSRSYNC);
}

void startAllTimers()
{
  GTCCR &= ~_BV(TSM);
}


TimerConfig getTimerConfig(uint8_t timer)
{
  volatile uint8_t *ptccra = getTimerTCCRA(timer);
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  if (!ptccra)
  {
    return {0, 0};
  }

  return {
    *ptccra,
    *ptccrb
  };
}

void restoreTimerConfig(uint8_t timer, TimerConfig config)
{
  volatile uint8_t *ptccra = getTimerTCCRA(timer);
  volatile uint8_t *ptccrb = getTimerTCCRB(timer);

  if (!ptccra)
  {
    return;
  }

  uint8_t tccraVal = (*ptccra & 0b11111100) | (config.tccra & 0b00000011);
  uint8_t tccrbVal = config.tccrb;

  *ptccra = tccraVal;
  *ptccrb = tccrbVal;
}