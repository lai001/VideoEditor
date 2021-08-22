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

#include "AudioTrack.h"
#include "VideoDescription.h"

FAudioTrack::FAudioTrack()
{
    audioBuffers = QVector<FAudioBuffer*>();
}

FAudioTrack::~FAudioTrack()
{
    if (decoder)
    {
        delete decoder;
        decoder = nullptr;
    }
    requestCleanAllCache();
}

void FAudioTrack::prepare(const FVideoDescription& videoDescription)
{
    if (decoder)
    {
        delete decoder;
        decoder = nullptr;
    }
    QAudioFormat format = videoDescription.audioFormat;
    decoder = new FAudioDecoder(filePath, format);
    audioFormat = format;
}

void FAudioTrack::requestCleanAllCache()
{
    QMutexLocker locker(&decoderMutex);
    for (auto item : audioBuffers)
    {
        delete item;
    }
    audioBuffers.clear();
}

void FAudioTrack::onSeeking(const FMediaTime& time)
{
    if (decoder)
    {
        QMutexLocker locker(&decoderMutex);
        decoder->seek(time);
    }
}

void FAudioTrack::samples(const FMediaTimeRange& timeRange, const int byteCount, uint8_t* buffer, const QAudioFormat& format)
{

    QMutexLocker locker(&decoderMutex);

    uint8_t* bufferArray = nullptr;

    int bufferStart = 0;
    int bufferEnd = 0;

    const int start = timeRange.start.convertScale(format.sampleRate()).timeValue();
    const int end = timeRange.end.convertScale(format.sampleRate()).timeValue();

    while (true)
    {
        if (audioBuffers.isEmpty() == false)
        {
            bufferStart = audioBuffers.first()->timeRange.start.convertScale(format.sampleRate()).timeValue();
            bufferEnd = audioBuffers.last()->timeRange.end.convertScale(format.sampleRate()).timeValue();

            if (bufferStart <= start && bufferEnd >= end)
            {
                int length = 0 ;
                for (auto item : audioBuffers)
                {
                    length += item->byteCount;
                }
                bufferArray = new uint8_t[length];
                int index = 0;
                for (auto item : audioBuffers)
                {
                    memcpy(bufferArray + index, item->pcmBuffer, item->byteCount);
                    index += item->byteCount;
                }
                const int sampleSize = format.sampleSize() / sizeof(uint8_t) / 8;

                int offset = (start - bufferStart) * format.channelCount() * sampleSize;

                if (offset < length)
                {
//                    qDebug() << "copy success" << offset << length;
                    memcpy(buffer, bufferArray + offset, byteCount);
                }
                else
                {
                    qDebug() << "copy failed" << offset << length;
                }

                delete[] bufferArray;
                break;
            }
            else
            {
                FAudioBuffer* audioBuffer = new FAudioBuffer();
                int ret = decoder->frame(audioBuffer); // TODO:
//                qDebug() << audioBuffers.size() << audioBuffer->timeRange.start.seconds();
                audioBuffers.append(audioBuffer);
            }
        }
        else
        {
            FAudioBuffer* audioBuffer = new FAudioBuffer();
            int ret = decoder->frame(audioBuffer); // TODO:
//            qDebug() << audioBuffers.size() << audioBuffer->timeRange.start.seconds();
            audioBuffers.append(audioBuffer);
        }
    }

}
