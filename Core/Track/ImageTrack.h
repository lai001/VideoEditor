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

#ifndef FIMAGETRACK_H
#define FIMAGETRACK_H

#include <QString>
#include "Image.h"
#include "FTime.h"

class FVideoDescription;

class FImageTrack
{
public:
    virtual ~FImageTrack() = 0;

public:
    FMediaTimeMapping timeMapping;
    QString filePath;

    virtual const FImage *sourceFrame(FMediaTime time, QSize renderSize, float renderScale) = 0;
    virtual FImage *compositionImage(const FImage *sourceFrame, const FMediaTime compositionTime, const QSize renderSize, const float renderScale) = 0;

    virtual void prepare(const FVideoDescription& videoDescription) = 0;
    virtual void didReloadFrame(const FVideoDescription& videoDescription) = 0;

    virtual void requestCleanCache(const FMediaTimeRange timeRange) = 0;
    virtual void requestCleanAllCache() = 0;
    virtual void onSeeking(const FMediaTime time) = 0;
};

#endif // FIMAGETRACK_H
