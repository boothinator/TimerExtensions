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

#define HIGH 0x1
#define LOW  0x0

static bool ticksInRangeExclusive(ticksExtraRange_t ticks, ticksExtraRange_t start, ticksExtraRange_t end)
{
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

PulseGen::PulseGen(volatile uint8_t *_pinReg, volatile uint8_t *_ddr,
    volatile uint16_t *_ocr, volatile uint8_t *_tccra, volatile uint8_t *_tccrc,
    uint8_t _pinBit, uint8_t _ddBit, uint8_t _com1, uint8_t _com0, uint8_t _foc,
    uint16_t prescalerDivisor, ExtTimer *_tcnt, stateChangeCallback_t _cb, void *_cbData)
{
  assert(_pinReg && _ddr && _ocr && _tccra && _tccrc && prescalerDivisor && _tcnt);

  this->pinReg = _pinReg;
  this->ddr = _ddr;
  this->ocr = _ocr;
  this->tccra = _tccra;
  this->tccrc = _tccrc;
  this->pinBit = _pinBit;
  this->ddBit = _ddBit;
  this->com1 = _com1;
  this->com0 = _com0;
  this->foc = _foc;
  this->tcnt = _tcnt;

  minChangeTicks = minPulseChangeCycles / prescalerDivisor;
  
  this->cb = _cb;
  this->cbData = _cbData;
}

bool PulseGen::setStart(ticksExtraRange_t _start)
{
  // Not enough time to update, or we've already gone high
  if ((PulseState::WaitingToScheduleHigh == pulseState || PulseState::ScheduledHigh == pulseState)
    && hasTimeToUpdate(start) == false)
  {
    return false;
  }

  char prevSREG = SREG;
  cli();

  this->start = _start;

  SREG = prevSREG; // restore interrupt state of the caller

  return true;
}

bool PulseGen::setEnd(ticksExtraRange_t _end)
{
  // Not enough time to update
  if (hasTimeToUpdate(end) == false)
  {
    return false;
  }

  // Can't respond to interrupt fast enough to schedule the end time
  if ((_end - start) < minChangeTicks)
  {
    return false;
  }

  char prevSREG = SREG;
  cli();

  this->end = _end;

  // Only change state if idle
  if (PulseState::Idle == pulseState)
  {

    // Set to clear bit on compare
    // Ensure that whole register gets set at the same time (TODO: does the compiler actually do this correctly?)
    uint8_t tmp = (*tccra | _BV(com1)) & ~_BV(com0);
    *tccra = tmp;

    // Force output compare to ensure it's low
    *tccrc |= _BV(foc);

    // Make pin output
    *ddr |= _BV(ddBit);

    // Use the compare interrupt to check if it's time to schedule,
    // but give us plenty of time to schedule by checking
    // UINT16_MAX - 1 ticks before the event
    *ocr = ((ticks16_t) start) + 1;

    pulseState = PulseState::WaitingToScheduleHigh;
    if (cb)
    {
      cb(this, const_cast<void *>(cbData));
    }

    updateState();
  }

  SREG = prevSREG; // restore interrupt state of the caller

  return true;
}

const ticksExtraRange_t PulseGen::getStart()
{
  return start;
}

const ticksExtraRange_t PulseGen::getEnd()
{
  return end;
}

// Only called from ISR (except for testing)
void PulseGen::processCompareEvent()
{
  updateState();
}

// Must be called from ISR or in critical section
void PulseGen::updateState()
{
  if (PulseState::Idle == pulseState)
  {
    // Don't use this function to transition from Idle to other states
  }
  else if (PulseState::WaitingToScheduleHigh == pulseState)
  {
    if (ticksInScheduleRange(start))
    {
      scheduleHighState();
    }
  }
  else if (PulseState::ScheduledHigh == pulseState)
  {
    // Use the compare interrupt to check if it's time to schedule,
    // but give us plenty of time to schedule by checking
    // UINT16_MAX - 1 ticks before the event
    *ocr = ((ticks16_t)end) + 1;

    pulseState = PulseState::WaitingToScheduleLow;
    if (cb)
    {
      cb(this, const_cast<void *>(cbData));
    }

    if (ticksInScheduleRange(end))
    {
      scheduleLowState();
    }
  }
  else if (PulseState::WaitingToScheduleLow == pulseState)
  {
    if (ticksInScheduleRange(end))
    {
      scheduleLowState();
    }
  }
  else if (PulseState::ScheduledLow == pulseState)
  {
    // Right now low state == end state, so go idle when we're scheduled low and get a compare event
    pulseState = PulseState::Idle;
    if (cb)
    {
      cb(this, const_cast<void *>(cbData));
    }
  }
}

// Must be called from ISR or in critical section
void PulseGen::scheduleHighState()
{
  *ocr = (ticks16_t) start;

  // Set bit on compare match
  *tccra |= _BV(com0);

  pulseState = PulseState::ScheduledHigh;
  if (cb)
  {
    cb(this, const_cast<void *>(cbData));
  }
}

// Must be called from ISR or in critical section
void PulseGen::scheduleLowState()
{
  *ocr = (ticks16_t) end;

  // Clear bit on compare match
  *tccra &= ~_BV(com0);

  pulseState = PulseState::ScheduledLow;
  if (cb)
  {
    cb(this, const_cast<void *>(cbData));
  }
}

const uint8_t PulseGen::getPinState()
{
	if (*pinReg & _BV(pinBit))
  {
    return HIGH;
  }
  else
  {
	  return LOW;
  }
}

// Critical section not needed because PulseState is a single byte
// and can be retrieved with a single instruction
const PulseGen::PulseState PulseGen::getState()
{
  return pulseState;
}

void PulseGen::setStateChangeCallback(stateChangeCallback_t _cb, const void *_cbData)
{
  char prevSREG = SREG;
  cli();

  this->cb = _cb;
  this->cbData = _cbData;

  SREG = prevSREG; // restore interrupt state of the caller
}

bool PulseGen::cancel()
{
  // Already idle?
  if (PulseState::Idle == pulseState) {
    return true;
  }

  // Can't cancel if pulse has started
  if (getPinState() == HIGH)
  {
    return false;
  }

  if (hasTimeToUpdate(start))
  {
    // Clear bit on compare match
    *tccra &= ~_BV(com0);
  
    pulseState = PulseState::Idle;
    if (cb)
    {
      cb(this, const_cast<void *>(cbData));
    }
    return true;
  }
  else
  {
      // Not enough time to cancel
      return false;
    }
  }

const bool PulseGen::ticksInScheduleRange(ticksExtraRange_t ticks)
{
  ticksExtraRange_t curTcnt = tcnt->get();
  ticksExtraRange_t firstTcntAfterRangeEnd = curTcnt + (1UL << 16);

  return ticksInRangeExclusive(ticks, curTcnt, firstTcntAfterRangeEnd);
}


const bool PulseGen::hasTimeToUpdate(ticksExtraRange_t curTicksSetting)
{
  if (ticksInRangeExclusive(tcnt->get(), curTicksSetting - minChangeTicks, curTicksSetting))
  {
    // Not enough time to update
    return false;
  }
  else 
  {
    return true;
  }
}