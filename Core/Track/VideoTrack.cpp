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

#include "VideoTrack.h"
#include <QFileInfo>

FVideoTrack::FVideoTrack()
{
    videoFrameQueue = QVector<FVideoFrame *>();
}

FVideoTrack::~FVideoTrack()
{
    if (decoder)
    {
        delete decoder;
    }
    requestCleanAllCache();
}

const FImage *FVideoTrack::sourceFrame(FMediaTime time, QSize renderSize, float renderScale)
{
    QMutexLocker locker(&decoderMutex);
    if (decoder == nullptr)
    {
        return nullptr;
    }

    FMediaTime fps = decoder->fps().convertScale(600);
    FMediaTime displayTime = time.convertScale(600);
    int r = displayTime.seconds() / (1.0 / fps.seconds());
    displayTime = FMediaTime((double)r * (1.0 / fps.seconds()), 600);

    for (FVideoFrame* videoFrame : videoFrameQueue)
    {
        if (videoFrame->displayTime == displayTime)
        {
            return videoFrame->image();
        }
    }

    FVideoFrame *videoFrame = new FVideoFrame();

    int ret = decoder->frameAtTime(displayTime, videoFrame);

    videoFrame->displayTime = displayTime;

    if (ret < 0)
    {
        delete videoFrame;
        videoFrame = nullptr;
    }

    if (videoFrame)
    {
        videoFrameQueue.append(videoFrame);
        return videoFrame->image();
    }
    return nullptr;
}

FImage *FVideoTrack::compositionImage(FImage *sourceFrame, FMediaTime compositionTime, QSize renderSize, float renderScale)
{
    return nullptr;
}

void FVideoTrack::prepare(FVideoDescription *videoDescription)
{
    if (decoder)
    {
        delete decoder;
    }
    decoder = new FVideoDecoder(filePath);
    //    qDebug() << decoder->fps() << QFileInfo(filePath).baseName();
}

void FVideoTrack::didReloadFrame(FVideoDescription *videoDescription)
{
}

void FVideoTrack::requestCleanCache(FMediaTimeRange timeRange)
{
    QMutexLocker locker(&decoderMutex);
    FMediaTimeRange cleanTimeRange = timeRange;
    FMediaTime fps = decoder->fps().convertScale(600);
    FMediaTime displayTime = cleanTimeRange.end.convertScale(600);
    int r = displayTime.seconds() / (1.0 / fps.seconds());
    cleanTimeRange.end = FMediaTime((double)r * (1.0 / fps.seconds()), 600);
    cleanTimeRange.end = cleanTimeRange.end - FMediaTime(1, fps.seconds());
    videoFrameQueue.erase(std::remove_if(videoFrameQueue.begin(), videoFrameQueue.end(), [&](FVideoFrame *videoFrame) {
                              if (cleanTimeRange.containsTime(videoFrame->displayTime))
                              {
                                  delete videoFrame;
                                  return true;
                              }
                              else
                              {
                                  return false;
                              }
                          }),
            videoFrameQueue.end());
}

void FVideoTrack::onSeeking(FMediaTime time)
{
    if (decoder)
    {
        QMutexLocker locker(&decoderMutex);
        decoder->seek(time);
    }
}

void FVideoTrack::requestCleanAllCache()
{
    QMutexLocker locker(&decoderMutex);
    for (auto item : videoFrameQueue)
    {
        delete item;
    }
    videoFrameQueue.clear();
}
