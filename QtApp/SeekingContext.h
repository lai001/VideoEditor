// Copyright (C) 2021 lmc
// 
// This file is part of VideoEditor.
// 
// VideoEditor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
// 
// VideoEditor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VideoEditor.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FSEEKINGCONTEXT_H
#define FSEEKINGCONTEXT_H

#include <mutex>
#include "FTime.h"

class FSeekingContext
{
public:
    FSeekingContext();

private:
    std::mutex lastSeekTimeMutex;
	std::mutex traceSeekTimeMutex;
    FMediaTime lastSeekTime = FMediaTime::zero;
    FMediaTime traceSeekTime = FMediaTime::zero;

public:
    void setLastSeekTime(FMediaTime time);
    FMediaTime getLastSeekTime();
    void setTraceSeekTime(FMediaTime time);
    FMediaTime getTraceSeekTime();

};

#endif // FSEEKINGCONTEXT_H
