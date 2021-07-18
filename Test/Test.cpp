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

#include "Test.h"

#include "VideoDescription.h"
#include <QtDebug>

Test::Test()
{
    case1();
    case2();
    case3();
}

void Test::case1()
{
    qDebug() << "case1";
    QVector<FMediaTimeRange> testRanges;

    testRanges.append(FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(1.0, 600)));
    testRanges.append(FMediaTimeRange(FMediaTime(0.5, 600), FMediaTime(2.0, 600)));
    testRanges.append(FMediaTimeRange(FMediaTime(10000.0, 600), FMediaTime(100.0, 600)));

    testRanges = FVideoDescription::instructionTimeRanges(testRanges);

    for (auto timeRange : testRanges)
    {
        qDebug() << timeRange.debugDescription();
    }
}

void Test::case2()
{
    qDebug() << "case2";
    FMediaTime time0 = FMediaTime(4410, 44100);
    FMediaTime time1 = FMediaTime(1, 600);

    assert(time0 > time1);
}

void Test::case3()
{
    qDebug() << "case3";
    FMediaTime time0 = FMediaTime(4410, 44100 * 600);
    FMediaTime time1 = FMediaTime(4410, 44100 * 600);

    assert(time0 == time1);
}
