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

#ifndef FPLAYER_H
#define FPLAYER_H

#include <functional>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QAudioOutput>

#include "VideoDescription.h"
#include "Thread.h"
#include "VideoFrameReceiver.h"
#include "PlayerImageQueueItem.h"
#include "AudioDecoder.h"
#include "SeekingContext.h"

class FPlayer : public QObject
{
    Q_OBJECT
public:
    enum class Status
    {
        playing,
        pause,
        seeking
    };

public:
    FPlayer();
    ~FPlayer();

public:
    FVideoDescription *videoDescription = nullptr;

    void play();
    void pause();
    void seek(FMediaTime time, std::function<void()> completion);
    FPlayer::Status status() const;
    void setReceiver(FVideoFrameReceiver *receiver);

private:
    FPlayer::Status _status = FPlayer::Status::pause;
    FMediaTime playTime;
    QDateTime startPlayTime;
    FMediaTime systemTime();

// video
private:
    FMediaTime decodeImageTime = FMediaTime(0, 600);
    FVideoFrameReceiver *receiver = nullptr;
    QTimer *imagetOutputTimer = nullptr;
    FThread *decodeImageThread = nullptr;
    QMutex shouldDecodeImageMutex;
    bool _shouldDecodeImage = false;
    FSeekingContext videoSeekingContext;
    QVector<const FPlayerImageQueueItem *> imageQueue;
    QMutex imageQueueMutex;

    void displayImage(const FPlayerImageQueueItem *queueItem);
    void openDecodeImageThread();
    void enqueueImageItem(FPlayerImageQueueItem * imageItem);
    const FPlayerImageQueueItem* dequeueImageItem();
    void cleanImageQueue();
    int getQueueSize();
    bool getShouldDecodeImage();
    void setShouldDecodeImage(bool flag);

// audio
private:
    FMediaTime decodeAudioTime = FMediaTime(0, 44100);
    QAudioOutput* audioOutput = nullptr;
    FSeekingContext audioSeekingContext;
    FThread* audioThread = nullptr;
    QIODevice* io = nullptr;
    QMutex shouldDecodeAudioMutex;
    bool _shouldDecodeAudio = false;

    bool initAudio();
    bool getShouldDecodeAudio();
    void setShouldDecodeAudio(bool flag);
    void openAudioThread();

private slots:
    void imagetOutputTimerTick();
};

#endif // FPLAYER_H
