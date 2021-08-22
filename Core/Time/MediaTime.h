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

#ifndef FMEDIATIME_H
#define FMEDIATIME_H

#include <QString>
#include "Vendor/FFmpeg.h"

struct FMediaTime
{
public:
    FMediaTime();
    FMediaTime(const double seconds, const int timeScale);
    FMediaTime(const int timeValue, const int timeScale);
    FMediaTime(const AVRational rational);

public:
    double seconds() const;
    const AVRational getRational() const;
    int timeValue() const;
    int timeScale() const;

    FMediaTime add(const FMediaTime time) const;
    FMediaTime subtract(const FMediaTime time) const;
    FMediaTime muliply(const FMediaTime time) const;
    FMediaTime convertScale(const int timeScale) const;
    FMediaTime nearer(const FMediaTime time0, const FMediaTime time1) const;
    FMediaTime invert() const;

    QString debugDescription() const;

    /**
     * operator
     */
    FMediaTime operator+(const FMediaTime &time);
    FMediaTime operator-(const FMediaTime &time);
    bool operator<(const FMediaTime &time) const;
    bool operator>(const FMediaTime &time) const;
    bool operator==(const FMediaTime &time) const;
    bool operator!=(const FMediaTime &time) const;
    bool operator<=(const FMediaTime &time) const;
    bool operator>=(const FMediaTime &time) const;

public:
    static FMediaTime zero;

private:
    AVRational rational;
};

#endif // FMEDIATIME_H
