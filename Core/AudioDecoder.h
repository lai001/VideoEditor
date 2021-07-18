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

#ifndef FAUDIODECODER_H
#define FAUDIODECODER_H

#include <QAudioFormat>
#include <QString>
#include <QMutex>
#include "FTime.h"
#include "FFmpeg.h"
#include "AudioBuffer.h"


class FAudioDecoder
{
public:
    FAudioDecoder(QString filePath, QAudioFormat format);
    ~FAudioDecoder();

public:
    QString filePath = "";

    SwrContext *swrctx = nullptr;
    AVFormatContext *formatContext = nullptr;
    AVStream *audioStream = nullptr;
    AVCodecContext *audioCodecCtx = nullptr;
    AVCodec *codec = nullptr;
    int audioStreamIndex = -1;

public:
    int frame(FAudioBuffer* audioBuffer);
    int seek(FMediaTime time);

    FMediaTime lastDecodedAudioChunkDisplayTime() const;
    FMediaTime fps() const;

private:
    AVSampleFormat outputSampleFormat = AV_SAMPLE_FMT_NONE;
    QMutex mutex;
    FMediaTime _lastDecodedAudioChunkDisplayTime = FMediaTime::zero;
};

#endif // FAUDIODECODER_H
