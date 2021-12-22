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

#ifndef TIMER_EXT_TIMER_UTIL_H_
#define TIMER_EXT_TIMER_UTIL_H_

#include <stdint.h>

#include "timerTypes.h"

enum class TimerClock { None, Clk, ClkDiv8, ClkDiv32, ClkDiv64, ClkDiv128, ClkDiv256, ClkDiv1024 };

enum class TimerMode { Normal /* TODO: implement others*/ };

bool configureTimerClock(uint8_t timer, TimerClock clock);

bool configureTimerMode(uint8_t timer, TimerMode mode);

uint8_t inputCapturePinToTimer(uint8_t pin);

void setInputCaptureNoiseCancellerEnabled(uint8_t timer, bool enabled);
uint8_t getInputCaptureNoiseCancellerEnabled(uint8_t timer);

bool hasInputCapture(uint8_t timer);
void clearInputCapture(uint8_t timer);
void setInputCaptureEdge(uint8_t timer, uint8_t edge);
ticks16_t getInputCapture(uint8_t timer);

int clockCyclesPerTick(TimerClock clock);

int ticksToClockCycles(ticksExtraRange_t ticks, TimerClock clock);
int ticksToMilliseconds(ticksExtraRange_t ticks, TimerClock clock);
int ticksToMicroseconds(ticksExtraRange_t ticks, TimerClock clock);

ticksExtraRange_t clockCyclesToTicks(uint32_t clockCycles, TimerClock clock);
ticksExtraRange_t millisecondsToTicks(uint32_t milliseconds, TimerClock clock);
ticksExtraRange_t microsecondsToTicks(uint32_t microseconds, TimerClock clock);

ticks16_t getTimerValue(uint8_t timer);
void setTimerValue(uint8_t timer, ticks16_t ticks);

struct TimerConfig
{
  uint8_t tccra;
  uint8_t tccrb;
};

TimerConfig getTimerConfig(uint8_t timer);
void restoreTimerConfig(uint8_t timer, TimerConfig config);

#endif // TIMER_EXT_TIMER_UTIL_H_
