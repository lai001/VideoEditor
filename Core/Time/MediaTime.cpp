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

#include "MediaTime.h"
#include <QObject>

FMediaTime::FMediaTime()
    : rational(FMediaTime::zero.getRational())
{
}

FMediaTime::FMediaTime(const double seconds, const int timeScale)
{
    rational.num = seconds * double(timeScale);
    rational.den = timeScale;
}

FMediaTime::FMediaTime(const int timeValue, const int timeScale)
{
    rational.num = timeValue;
    rational.den = timeScale;
}

FMediaTime::FMediaTime(const AVRational rational)
    : rational(rational)
{
}

double FMediaTime::seconds() const
{
    return av_q2d(rational);
}

const AVRational FMediaTime::getRational() const
{
    return rational;
}

int FMediaTime::timeValue() const
{
    return rational.num;
}

int FMediaTime::timeScale() const
{
    return rational.den;
}

FMediaTime FMediaTime::add(const FMediaTime time) const
{
    FMediaTime newTime = FMediaTime(av_add_q(rational, time.rational));
    return newTime;
}

FMediaTime FMediaTime::subtract(const FMediaTime time) const
{
    FMediaTime newTime = FMediaTime(av_sub_q(rational, time.rational));
    return newTime;
}

FMediaTime FMediaTime::muliply(const FMediaTime time) const
{
    FMediaTime newTime = FMediaTime(av_mul_q(rational, time.rational));
    return newTime;
}

FMediaTime FMediaTime::convertScale(const int timeScale) const
{
    FMediaTime newTime = FMediaTime((int)((double)timeValue() / (double)this->timeScale() * (double)timeScale), timeScale);
    return newTime;
}

FMediaTime FMediaTime::nearer(const FMediaTime time0, const FMediaTime time1) const
{
    int compareResult = av_nearer_q(getRational(), time0.getRational(), time1.getRational());
    if (compareResult == 1)
    {
        return FMediaTime(time0);
    }
    else if (compareResult == -1)
    {
        return FMediaTime(time1);
    }
    else
    {
        return *this;
    }
}

FMediaTime FMediaTime::invert() const
{
    return FMediaTime(av_inv_q(this->getRational()));
}

QString FMediaTime::debugDescription() const
{
    QString t;
    t.sprintf("%8p", this);
    return QString("<%1> seconds: %2").arg(t).arg(seconds());
}

FMediaTime FMediaTime::operator+(const FMediaTime &time)
{
    return this->add(time);
}

FMediaTime FMediaTime::operator-(const FMediaTime &time)
{
    return this->subtract(time);
}

bool FMediaTime::operator<(const FMediaTime &time) const
{
    return av_cmp_q(this->getRational(), time.getRational()) == -1;
}

bool FMediaTime::operator>(const FMediaTime &time) const
{
    return av_cmp_q(this->getRational(), time.getRational()) == 1;
}

bool FMediaTime::operator==(const FMediaTime &time) const
{
    return av_cmp_q(this->getRational(), time.getRational()) == 0;
}

bool FMediaTime::operator!=(const FMediaTime &time) const
{
    return (*this == time) == false;
}

bool FMediaTime::operator<=(const FMediaTime &time) const
{
    return *this < time || *this == time;
}

bool FMediaTime::operator>=(const FMediaTime &time) const
{
    return *this > time || *this == time;
}

FMediaTime FMediaTime::zero = FMediaTime(0.0, 600);
