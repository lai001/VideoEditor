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

#ifndef VIDEOTRACK_H
#define VIDEOTRACK_H

#include <QMutex>

#include "ImageTrack.h"
#include "VideoDecoder.h"

class FVideoTrack : public FImageTrack
{
public:
    FVideoTrack();
    ~FVideoTrack();

    virtual const FImage *sourceFrame(FMediaTime time, QSize renderSize, float renderScale) override;
    virtual FImage *compositionImage(const FImage *sourceFrame, const FMediaTime compositionTime, const QSize renderSize, const float renderScale) override;

    virtual void prepare(const FVideoDescription& videoDescription) override;
    virtual void didReloadFrame(const FVideoDescription& videoDescription) override;

    virtual void requestCleanCache(const FMediaTimeRange timeRange) override;
    void requestCleanAllCache() override;
    void onSeeking(const FMediaTime time) override;

private:
    FVideoDecoder *decoder = nullptr;

    QVector<FVideoFrame *> videoFrameQueue;

    QMutex decoderMutex;
};

#endif // VIDEOTRACK_H
