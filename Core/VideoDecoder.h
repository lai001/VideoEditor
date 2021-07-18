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

#ifndef FVIDEODECODER_H
#define FVIDEODECODER_H

#include <QString>
#include <QImage>
#include <QPixmap>
#include <QMutex>
#include "FTime.h"
#include "VideoFrame.h"
#include "FFmpeg.h"

class FVideoDecoder
{
public:
    FVideoDecoder(QString filePath);
    ~FVideoDecoder();

private:
    QString filePath = "";

    AVFormatContext *formatContext = nullptr;
    AVStream *videoStream = nullptr;
    AVCodecContext *videoCodecCtx = nullptr;
    AVCodec *codec = nullptr;
    int videoStreamIndex = -1;
    struct SwsContext *imageSwsContext = nullptr;
    FMediaTime _lastDecodedImageDisplayTime = FMediaTime::zero;

    QMutex mutex;

public:
    int frameAtTime(FMediaTime time, FVideoFrame *videoFrame);
    int seek(FMediaTime time);

    FMediaTime lastDecodedImageDisplayTime();
    FMediaTime fps();
};

#endif // FVIDEODECODER_H
