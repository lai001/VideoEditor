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

#ifndef FAUDIOTRACK_H
#define FAUDIOTRACK_H

#include <QMutex>
#include <QAudioFormat>
#include <QVector>

#include "FTime.h"
#include "AudioDecoder.h"
#include "AudioBuffer.h"

class FVideoDescription;

class FAudioTrack
{
public:
    FAudioTrack();
    ~FAudioTrack();

public:
    FMediaTimeMapping timeMapping;
    QString filePath;
    void prepare(const FVideoDescription& videoDescription);
    void didReloadFrame(const FVideoDescription& videoDescription);

    void requestCleanCache(const FMediaTimeRange& timeRange);
    void requestCleanAllCache();
    void onSeeking(const FMediaTime& time);

    void samples(const FMediaTimeRange& timeRange, const int byteCount, uint8_t* buffer, const QAudioFormat& format);

private:
    FAudioDecoder* decoder = nullptr;
    QAudioFormat audioFormat;

    QVector<FAudioBuffer*> audioBuffers;

private:
     QMutex decoderMutex;
};

#endif // FAUDIOTRACK_H
