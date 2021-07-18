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

#include "Player.h"

#include <climits>
#include <QPainter>
#include <QApplication>

#include "ScopeGuard.h"
#include "Image.h"
#include "Util.h"

FPlayer::FPlayer()
{

}

FPlayer::~FPlayer()
{    
    setShouldDecodeImage(false);
    setShouldDecodeAudio(false);

    if (imagetOutputTimer)
    {
        if (imagetOutputTimer->isActive())
        {
            imagetOutputTimer->stop();
        }
        delete imagetOutputTimer;
        imagetOutputTimer = nullptr;
    }

    if (decodeImageThread)
    {
        delete decodeImageThread;
        decodeImageThread = nullptr;
    }

    if (audioThread)
    {
        delete audioThread;
        audioThread = nullptr;
    }

    if (audioOutput)
    {
        delete audioOutput;
        audioOutput = nullptr;
    }

    if (io)
    {
        delete io;
        io = nullptr;
    }

    cleanImageQueue();
}

void FPlayer::play()
{
    startPlayTime = QDateTime::currentDateTime();

    if (imagetOutputTimer)
    {
        if (imagetOutputTimer->isActive() == false)
        {
            imagetOutputTimer->start();
        }
    }

    openDecodeImageThread();

    if (audioOutput)
    {
        switch (audioOutput->state()) {
        case QAudio::ActiveState:
            break;
        case QAudio::IdleState:
            break;
        case QAudio::InterruptedState:
            break;
        case QAudio::StoppedState:
            io = audioOutput->start();
            break;
        case QAudio::SuspendedState:
            audioOutput->resume();
            break;
        }
    }

    openAudioThread();
}

void FPlayer::pause()
{
    if (imagetOutputTimer)
    {
        if (imagetOutputTimer->isActive())
        {
            imagetOutputTimer->stop();
        }
    }

    if (audioOutput)
    {
        switch (audioOutput->state()) {
        case QAudio::ActiveState:
            audioOutput->suspend();
            break;
        case QAudio::IdleState:
            break;
        case QAudio::InterruptedState:
            break;
        case QAudio::StoppedState:
            break;
        case QAudio::SuspendedState:
            break;
        }
    }
}

void FPlayer::seek(FMediaTime time, std::function<void()> completion)
{
    videoSeekingContext.setTraceSeekTime(time);
    audioSeekingContext.setTraceSeekTime(time);
    playTime = time;
}

FPlayer::Status FPlayer::status() const
{
    return _status;
}

void FPlayer::setReceiver(FVideoFrameReceiver *receiver)
{
    if (videoDescription == nullptr)
    {
        return;
    }
    this->receiver = receiver;
    imagetOutputTimer = new QTimer();
    imagetOutputTimer->setTimerType(Qt::PreciseTimer);
    connect(imagetOutputTimer, SIGNAL(timeout()), this, SLOT(imagetOutputTimerTick()));
    //    int interval = FMediaTime(1, 60).seconds() * 1000.0;
    int interval = FMediaTime(1, videoDescription->fps).seconds() * 1000.0;
    imagetOutputTimer->setInterval(interval);
    imagetOutputTimer->start();
}

FMediaTime FPlayer::systemTime()
{
    QDateTime current = QDateTime::currentDateTime();

    qint64 msec = current.toMSecsSinceEpoch();
    double seconds = (double)msec / 1000.0;
    return FMediaTime(seconds, 600);
}

bool FPlayer::getShouldDecodeImage()
{
    QMutexLocker locker(&shouldDecodeImageMutex);
    return _shouldDecodeImage;
}

void FPlayer::setShouldDecodeImage(bool flag)
{
    QMutexLocker locker(&shouldDecodeImageMutex);
    _shouldDecodeImage = flag;
}

void FPlayer::openDecodeImageThread()
{
    if (decodeImageThread == nullptr)
    {
        setShouldDecodeImage(true);
        decodeImageThread = new FThread();
        decodeImageThread->async([&]
        {
//            qDebug() << "create thread: " << QThread::currentThreadId();

            while (true)
            {
                if (getShouldDecodeImage() == false)
                {
                    break;
                }

                FMediaTime lastSeekTime = videoSeekingContext.getLastSeekTime();
                FMediaTime traceSeekTime = videoSeekingContext.getTraceSeekTime();
                bool isSeeking = lastSeekTime != traceSeekTime;
                if (isSeeking)
                {
                    cleanImageQueue();

                    for (auto imageTrack : videoDescription->imageTracks)
                    {
                        imageTrack->requestCleanAllCache();
                        imageTrack->onSeeking(traceSeekTime);
                    }

                    float fps = videoDescription->fps;
                    FMediaTime dTime = traceSeekTime.convertScale(600);
                    int r = dTime.seconds() / (1.0 / fps);
                    decodeImageTime = FMediaTime((double)r * (1.0 / fps), 600);
                    videoSeekingContext.setLastSeekTime(traceSeekTime);
                }

                FVideoInstruction *videoInstuction = videoDescription->videoInstuction(decodeImageTime);
                if (videoInstuction == nullptr)
                {
                    break;
                }

                if (getQueueSize() >= 5)
                {
                    QThread::currentThread()->msleep(1.0 / 24.0 * 1000.0);
                }
                else
                {
                    QSize renderSize = videoDescription->renderSize;
                    float renderScale = videoDescription->renderScale;

                    FPlayerImageQueueItem *queueItem = new FPlayerImageQueueItem(renderSize, QImage::Format_RGB32);
                    queueItem->displayTime = decodeImageTime;

                    QPainter* painter = new QPainter(queueItem->qimage());
                    defer {
                        delete painter;
                        painter = nullptr;
                    };

                    int index = 0;
                    for (FImageTrack *imageTrack : videoInstuction->imageTracks)
                    {
                        const FImage *image = imageTrack->sourceFrame(decodeImageTime, renderSize, renderScale);
                        if (index == 0)
                        {
                            painter->drawImage(QRect(0, 0, renderSize.width() / 2, renderSize.height() / 2),
                                               *image->image(),
                                               QRect(0, 0, image->image()->size().width(), image->image()->size().height()));
                        }
                        else
                        {
                            painter->drawImage(QRect(renderSize.width() / 2, renderSize.height() / 2, renderSize.width() / 2, renderSize.height() / 2),
                                               *image->image(),
                                               QRect(0, 0, image->image()->size().width(), image->image()->size().height()));
                        }
                        index++;
                    }

                    if (isSeeking)
                    {
                        FUtil::runInMainThread([this, queueItem]{
                            displayImage(queueItem);
                            delete queueItem;
                        });
                    }
                    else
                    {
                        enqueueImageItem(queueItem);
                    }

                    float fps = videoDescription->fps;
                    decodeImageTime = decodeImageTime + FMediaTime(1.0 / fps, 600.0);
                }
            }

            cleanImageQueue();

        }, [&]{
            delete decodeImageThread;
            decodeImageThread = nullptr;
        });
    }
}

void FPlayer::enqueueImageItem(FPlayerImageQueueItem *imageItem)
{
    QMutexLocker locker(&imageQueueMutex);
    imageQueue.append(imageItem);
}

const FPlayerImageQueueItem *FPlayer::dequeueImageItem()
{
    QMutexLocker locker(&imageQueueMutex);
    if (imageQueue.isEmpty() == false)
    {
        const FPlayerImageQueueItem *queueItem = imageQueue.first();
        imageQueue.erase(imageQueue.begin());
        return queueItem;
    }
    else
    {
        return nullptr;
    }
}

void FPlayer::cleanImageQueue()
{
    QMutexLocker locker(&imageQueueMutex);
    for (const FPlayerImageQueueItem *queueItem : imageQueue)
    {
        delete queueItem;
    }
    imageQueue.clear();
}

int FPlayer::getQueueSize()
{
    QMutexLocker locker(&imageQueueMutex);
    return imageQueue.size();
}

bool FPlayer::initAudio()
{
    if (audioOutput != nullptr)
    {
        return true;
    }
    if (videoDescription == nullptr)
    {
        return false;
    }
    if (videoDescription->audioTracks.isEmpty())
    {
        qDebug() << "No audio tracks.";
        return false;
    }

    QAudioFormat format = videoDescription->audioFormat;
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

    if (!info.isFormatSupported(format))
    {
        qDebug() << "Not support this audio format.";
        return false;
    }
    audioOutput = new QAudioOutput(info, format);
    io = audioOutput->start();
    qDebug() << "Init audio device success.";
    return true;
}

bool FPlayer::getShouldDecodeAudio()
{
    QMutexLocker locker(&shouldDecodeAudioMutex);
    return _shouldDecodeAudio;
}

void FPlayer::setShouldDecodeAudio(bool flag)
{
    QMutexLocker locker(&shouldDecodeAudioMutex);
    _shouldDecodeAudio = flag;
}

void FPlayer::openAudioThread()
{
    bool ret = initAudio();
    if (ret == false || audioThread != nullptr)
    {
        return;
    }
    qDebug("openAudioThread");
    setShouldDecodeAudio(true);
    audioThread = new FThread();
    audioThread->async([&]{

        int length = audioOutput->periodSize();
        uint8_t* outputBuffer = new uint8_t[length];
        defer {
            delete[] outputBuffer;
            outputBuffer = nullptr;
        };
        while (true)
        {
            if (getShouldDecodeAudio() == false)
            {
                break;
            }

            FMediaTime lastSeekTime = audioSeekingContext.getLastSeekTime();
            FMediaTime traceSeekTime = audioSeekingContext.getTraceSeekTime();
            bool isSeeking = lastSeekTime != traceSeekTime;
            if (isSeeking)
            {
                for (auto audioTrack : videoDescription->audioTracks)
                {
                    audioTrack->requestCleanAllCache();
                    audioTrack->onSeeking(traceSeekTime);
                }
                decodeAudioTime = traceSeekTime.convertScale(videoDescription->audioFormat.sampleRate());
                audioSeekingContext.setLastSeekTime(traceSeekTime);
//                continue;
            }

            FVideoInstruction *videoInstuction = videoDescription->videoInstuction(decodeAudioTime);
            if (videoInstuction == nullptr)
            {
                break;
            }

            if (audioOutput->bytesFree() < length)
            {
                QThread::msleep(10);
            }
            else
            {
                const int sampleSize = videoDescription->audioFormat.sampleSize() / sizeof(uint8_t) / 8;

                FMediaTime duration = FMediaTime(length / videoDescription->audioFormat.channelCount() / sampleSize, videoDescription->audioFormat.sampleRate());

                memset(outputBuffer, 0, length);
                uint8_t* buffer = new uint8_t[length];

                defer {
                    delete[] buffer;
                    buffer = nullptr;
                };

                FMediaTimeRange timeRange = FMediaTimeRange(decodeAudioTime, decodeAudioTime + duration);
//                qDebug() << timeRange.debugDescription();
                for (FAudioTrack* audioTrack : videoInstuction->audioTracks)
                {
                    memset(buffer, 0, length);
                    audioTrack->samples(timeRange, length, buffer, videoDescription->audioFormat);
                    for (int i = 0; i < length / sampleSize; i++)
                    {
                        // ((short*)outputBuffer)[i] = (((short*)outputBuffer)[i] + ((short*)buffer)[i]) / videoInstuction->audioTracks.size();
                        int mix = (((short*)outputBuffer)[i] + ((short*)buffer)[i]);
                        mix = qMin(mix, SHRT_MAX);
                        ((short*)outputBuffer)[i] = (short)mix ;
                    }
                }
                qint64 acualWrite = io->write((const char*)outputBuffer, length);
//                qDebug() << "acualWrite" << acualWrite << audioOutput->bufferSize();
                decodeAudioTime = decodeAudioTime + duration;
//                qDebug() << decodeAudioTime.debugDescription();
            }
        }
    }, [&]{
        delete audioThread;
        audioThread = nullptr;
    });
}

void FPlayer::imagetOutputTimerTick()
{
    qint64 delta = QDateTime::currentDateTime().toMSecsSinceEpoch() - startPlayTime.toMSecsSinceEpoch();
    double deltaSeconds = (double)delta / 1000.0;
    //        qDebug() << deltaSeconds;

    if (receiver)
    {
        const FPlayerImageQueueItem *queueItem = dequeueImageItem();

        if (queueItem)
        {
            displayImage(queueItem);

            for (auto imageTrack : videoDescription->imageTracks)
            {
                FMediaTimeRange cleanTimeRange = FMediaTimeRange(FMediaTime::zero, queueItem->displayTime);
                imageTrack->requestCleanCache(cleanTimeRange);
            }

            delete queueItem;
        }
        else
        {
            qDebug("No image to show");
        }
    }
    int fps = (int)videoDescription->fps;
    playTime = playTime + FMediaTime(1, fps);
    //        qDebug() << playTime.debugDescription();
}

void FPlayer::displayImage(const FPlayerImageQueueItem *queueItem)
{
    if (queueItem == nullptr)
    {
        return;
    }
    const FImage* image = queueItem->fimage();
    if (image)
    {
        if (image->image() != nullptr)
        {
            receiver->receiveFrame(image);
        }
        else
        {
            qDebug("No image to show");
        }
    }
    else
    {
        qDebug("No image to show");
    }
}
