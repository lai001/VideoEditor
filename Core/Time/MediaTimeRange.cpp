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

#include "MediaTimeRange.h"

FMediaTimeRange::FMediaTimeRange()
    : start(FMediaTime()), end(FMediaTime())
{
}

FMediaTimeRange::FMediaTimeRange(FMediaTime start, FMediaTime end)
    : start(start), end(end)
{
}

FMediaTime FMediaTimeRange::duration() const
{
    FMediaTime _end = end;
    return _end - start;
}

bool FMediaTimeRange::isEmpty() const
{
    return duration().seconds() <= 0.0;
}

FMediaTimeRange FMediaTimeRange::intersection(const FMediaTimeRange otherTimeRange) const
{
    FMediaTime _start = qMax(start, otherTimeRange.start);
    FMediaTime end = qMin(end, otherTimeRange.end);
    if (_start < end)
    {
        return FMediaTimeRange(_start, end);
    }
    else
    {
        return FMediaTimeRange();
    }
}

bool FMediaTimeRange::containsTime(const FMediaTime time) const
{
    if (time <= end && time >= start)
    {
        return true;
    }
    else
    {
        return false;
    }
}

QString FMediaTimeRange::debugDescription() const
{
    QString t;
    t.sprintf("%8p", this);
    return QString("<%1>, start: %2, end: %3").arg(t).arg(start.debugDescription()).arg(end.debugDescription());
}