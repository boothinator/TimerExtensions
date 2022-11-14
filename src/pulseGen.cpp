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

#include "pulseGen.h"

#include "avr/interrupt.h"
#include "assert.h"

#include "timerTypes.h"

#include <Arduino.h>

// Only allow setStart() and setEnd() if we are this many CPU cycles ahead of the new time
#ifndef MIN_PULSE_CHANGE_CYCLES
#define MIN_PULSE_CHANGE_CYCLES 256
#endif // MIN_PULSE_CHANGE_CYCLES

#ifndef EXTERNAL_CLOCK_FREQUENCY
#define EXTERNAL_CLOCK_FREQUENCY F_CPU
#endif // EXTERNAL_CLOCK_FREQUENCY

namespace {

static bool ticksInRangeExclusive(ticksExtraRange_t ticks, ticksExtraRange_t start, ticksExtraRange_t end)
{
  /*Serial.println("ticksInRangeExclusive");
  Serial.println(ticks);
  Serial.println(start);
  Serial.println(end);*/
  if (start < end)
  {
    return start < ticks && ticks < end;
  }
  else 
  {
    // Overflowed the extended range when we overflowed the sys range
    // start > end
    return start < ticks || ticks < end;
  }
}

constexpr ticks16_t extClockMinPulseChangeCycles =
 (ticks16_t)((uint64_t)MIN_PULSE_CHANGE_CYCLES * EXTERNAL_CLOCK_FREQUENCY) / F_CPU;

// Corresponds to the 3 clock select bits (CS02:0)
constexpr ticks16_t minChangeTicks[8] = {
  0, // No clock source
  MIN_PULSE_CHANGE_CYCLES / 1,    // No prescaler
  MIN_PULSE_CHANGE_CYCLES / 8,    // clk/8
  MIN_PULSE_CHANGE_CYCLES / 64,   // clk/64
  MIN_PULSE_CHANGE_CYCLES / 256,  // clk/256
  MIN_PULSE_CHANGE_CYCLES / 1024, // clk/1024
  extClockMinPulseChangeCycles,   // External clock
  extClockMinPulseChangeCycles    // External clock
};

constexpr ticks16_t getMinChangeTicks(uint8_t tccrb)
{
  return minChangeTicks[tccrb & 0b00000111] + 1;
}

} // namespace

PulseGen::PulseGen(volatile uint8_t *ocrl, volatile uint8_t *ocrh,
    volatile uint8_t *tccra, volatile uint8_t *tccrb, volatile uint8_t *tccrc, volatile uint8_t *timsk,
    uint8_t com1, uint8_t com0, uint8_t foc, uint8_t ocie, ExtTimer *tcnt) :
    _ocrl(ocrl), _ocrh(ocrh),
    _tccra(tccra), _tccrb(tccrb), _tccrc(tccrc), _timsk(timsk),
    _com1(com1), _com0(com0), _foc(foc), _ocie(ocie), _tcnt(tcnt)
{
  assert(_ocrl && _tccra && _tccrb && _tccrc && _tcnt);
}

bool PulseGen::setStart(ticksExtraRange_t start)
{
  // Not enough time to update, or we've already gone high
  if ((PulseState::WaitingToScheduleHigh == _pulseState || PulseState::ScheduledHigh == _pulseState)
    && hasTimeToUpdate(start) == false)
  {
    return false;
  }

  char prevSREG = SREG;
  cli();

  this->_start = start;

  SREG = prevSREG; // restore interrupt state of the caller

  return true;
}

bool PulseGen::setEnd(ticksExtraRange_t end)
{
  // Not enough time to update
  if (hasTimeToUpdate(end) == false)
  {
    return false;
  }

  // Can't respond to interrupt fast enough to schedule the end time
  if ((end - _start) < getMinChangeTicks(*_tccrb))
  {
    return false;
  }

  char prevSREG = SREG;
  cli();

  this->_end = end;

  SREG = prevSREG; // restore interrupt state of the caller

  return true;
}


void PulseGen::schedule()
{
  char prevSREG = SREG;
  cli();

  scheduleInternal();

  SREG = prevSREG; // restore interrupt state of the caller
}

void PulseGen::schedule(ticksExtraRange_t start, ticksExtraRange_t end)
{
  char prevSREG = SREG;
  cli();

  this->_start = start;
  this->_end = end;

  scheduleInternal();

  SREG = prevSREG; // restore interrupt state of the caller
}

ticksExtraRange_t PulseGen::getStart() const
{
  return _start;
}

ticksExtraRange_t PulseGen::getEnd() const
{
  return _end;
}

// Only called from ISR (except for testing)
void PulseGen::processCompareEvent()
{
  updateState();
}

// Must be called from ISR or in critical section
void PulseGen::updateState()
{
  if (PulseState::Idle == _pulseState)
  {
    // Don't use this function to transition from Idle to other states
  }
  else if (PulseState::WaitingToScheduleHigh == _pulseState)
  {
    if (ticksInScheduleRange(_start))
    {
      scheduleHighState();
    }
  }
  else if (PulseState::ScheduledHigh == _pulseState)
  {
    // Use the compare interrupt to check if it's time to schedule
    setOcr(getCheckTicks(_end));

    if (ticksInScheduleRange(_end))
    {
      scheduleLowState();
    }
    else
    {
      _pulseState = PulseState::WaitingToScheduleLow;
      if (_cb)
      {
        _cb(this, const_cast<void *>(_cbData));
      }
    }
  }
  else if (PulseState::WaitingToScheduleLow == _pulseState)
  {
    if (ticksInScheduleRange(_end))
    {
      scheduleLowState();
    }
  }
  else if (PulseState::ScheduledLow == _pulseState)
  {
    // Right now low state == end state, so go idle when we're scheduled low and get a compare event
    _pulseState = PulseState::Idle;
    
    // Disable OCR interrupt
    *_timsk &= ~_BV(_ocie);

    if (_cb)
    {
      _cb(this, const_cast<void *>(_cbData));
    }
  }
}

// Must be called from ISR or in critical section
void PulseGen::scheduleInternal()
{
  // Only change state if idle
  if (PulseState::Idle == _pulseState)
  {

    // Set to clear bit on compare
    // Ensure that whole register gets set at the same time
    uint8_t tmp = (*_tccra | _BV(_com1)) & ~_BV(_com0);
    *_tccra = tmp;

    // Force output compare to ensure it's low
    *_tccrc |= _BV(_foc);

    // Use the compare interrupt to check if it's time to schedule
    setOcr(getCheckTicks(_start));

    // Enable OCR interrupt
    *_timsk |= _BV(_ocie);

    _pulseState = PulseState::WaitingToScheduleHigh;
    if (_cb)
    {
      _cb(this, const_cast<void *>(_cbData));
    }

    updateState();
  }
}

// Must be called from ISR or in critical section
void PulseGen::scheduleHighState()
{
  setOcr((ticks16_t) _start);

  // Set bit on compare match
  *_tccra |= _BV(_com0);

  _pulseState = PulseState::ScheduledHigh;
  if (_cb)
  {
    _cb(this, const_cast<void *>(_cbData));
  }
}

// Must be called from ISR or in critical section
void PulseGen::scheduleLowState()
{
  setOcr((ticks16_t) _end);

  // Clear bit on compare match
  *_tccra &= ~_BV(_com0);

  _pulseState = PulseState::ScheduledLow;
  if (_cb)
  {
    _cb(this, const_cast<void *>(_cbData));
  }
}

// Critical section not needed because PulseState is a single byte
// and can be retrieved with a single instruction
PulseGen::PulseState PulseGen::getState() const
{
  return _pulseState;
}

int PulseGen::getTimer() const
{
  return _tcnt->getTimer();
}

ExtTimer *PulseGen::getExtTimer() const
{
  return _tcnt;
}

void PulseGen::setStateChangeCallback(stateChangeCallback_t _cb, const void *_cbData)
{
  char prevSREG = SREG;
  cli();

  this->_cb = _cb;
  this->_cbData = _cbData;

  SREG = prevSREG; // restore interrupt state of the caller
}

bool PulseGen::cancel()
{
  // Already idle?
  if (PulseState::Idle == _pulseState) {
    return true;
  }

  // Can't cancel if pulse has started
  if (pulseHasStarted())
  {
    return false;
  }

  if (hasTimeToUpdate(_start))
  {
    // Clear bit on compare match
    *_tccra &= ~_BV(_com0);
  
    _pulseState = PulseState::Idle;
    if (_cb)
    {
      _cb(this, const_cast<void *>(_cbData));
    }
    return true;
  }
  else
  {
      // Not enough time to cancel
      return false;
    }
  }

bool PulseGen::ticksInScheduleRange(ticksExtraRange_t ticks) const
{
  ticksExtraRange_t curTcnt = _tcnt->get();
  ticksExtraRange_t firstTcntAfterRangeEnd;

  if (_ocrh)
  {
    // 16-bit
    firstTcntAfterRangeEnd = curTcnt + (1UL << 16);
  }
  else
  {
    // 8-bit
    firstTcntAfterRangeEnd = curTcnt + (1UL << 8);
  }

  return ticksInRangeExclusive(ticks, curTcnt, firstTcntAfterRangeEnd);
}


bool PulseGen::hasTimeToUpdate(ticksExtraRange_t curTicksSetting) const
{
  if (ticksInRangeExclusive(_tcnt->get(), curTicksSetting - getMinChangeTicks(*_tccrb), curTicksSetting))
  {
    // Not enough time to update
    return false;
  }
  else 
  {
    return true;
  }
}

bool PulseGen::pulseHasStarted() const
{
  return ticksInRangeExclusive(_tcnt->get(), _start, _end);
}

void PulseGen::setOcr(ticks16_t val)
{
  if (_ocrh)
  {
    // 16-bit timer

    uint8_t high = static_cast<uint8_t>(val >> 8);
    uint8_t low = static_cast<uint8_t>(val);

    // Not needed: setOcr() is only called from critical sections
    //char prevSREG = SREG;
    //cli();
    
    // Follow correct 16-bit register access rules by storing the high register first
    *_ocrh = high;
    *_ocrl = low;

    //SREG = prevSREG; // restore interrupt state of the caller
  }
  else
  {
    // 8-bit timer
    *_ocrl = static_cast<uint8_t>(val);
  }
  
}

ticks16_t PulseGen::getCheckTicks(ticksExtraRange_t ticks) const
{
  // Use the compare interrupt to check if it's time to schedule,
  // but give us plenty of time to schedule by checking
  // UINT16_MAX - 1 or UINT8_MAX - 1 ticks before the event
  if (_ocrh)
  {
    // 16-bit
    uint16_t checkTicks = ((ticks16_t) _start) + 1;
    return checkTicks;
  }
  else
  {
    // 8-bit
    uint8_t checkTicks = ((ticks8_t) _start) + 1;
    return checkTicks;
  }
}