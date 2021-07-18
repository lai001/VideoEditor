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

#ifndef FMEDIATIMERANGE_H
#define FMEDIATIMERANGE_H

#include "MediaTime.h"
#include <QDebug>

struct FMediaTimeRange
{
public:
    FMediaTimeRange();
    FMediaTimeRange(FMediaTime start, FMediaTime end);

public:
    FMediaTime start;
    FMediaTime end;

    FMediaTime duration() const;
    bool isEmpty() const;

    FMediaTimeRange intersection(const FMediaTimeRange otherTimeRange) const;
    bool containsTime(const FMediaTime time) const;

    QString debugDescription() const;
};

#endif // FMEDIATIMERANGE_H
