// Extended Range Input Capture
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

#ifndef TIMER_EXT_EXT_CAPTURE_H_
#define TIMER_EXT_EXT_CAPTURE_H_

#include "timerTypes.h"

class ExtCapture
{
public:
  ExtCapture();

  ticksExtraRange_t get();

  void setInterruptEnabled(bool enabled);
  bool getInterruptEnabled();

  void setNoiseCancellerEnabled(bool enabled);
  bool getNoiseCancellerEnabled();

  bool hasCapture();
  void clearCaptureFlag();

  enum Edge {Rising, Falling};

  void setCaptureEdge(Edge edge);
  Edge getCaptureEdge();

  void processEvent(uint16_t value);
  
private:
  uint16_t value;
};


#endif // TIMER_EXT_EXT_CAPTURE_H_
