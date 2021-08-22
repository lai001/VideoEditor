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

#include "AudioPCMBuffer.h"

FAudioPCMBuffer::FAudioPCMBuffer(const QAudioFormat format, const int capacity)
    : _format(format), _capacity(capacity)
{
    assert(format.codec() == "audio/pcm");
    assert(format.sampleType() != QAudioFormat::SampleType::Unknown);

    channelData = new uint8_t *[format.channelCount()];
    for (int i = 0; i < format.channelCount(); i++)
    {
        int size = bytesDataSize();
        uint8_t *buffer = new uint8_t[size];
        channelData[i] = buffer;
    }

    timeRange = FMediaTimeRange(FMediaTime(0, format.sampleRate()), FMediaTime(capacity, format.sampleRate()));
}

FAudioPCMBuffer::FAudioPCMBuffer(const FAudioPCMBuffer &pcmBuffer)
{
    const QAudioFormat format = pcmBuffer.audioFormat();
    channelData = new uint8_t *[format.channelCount()];
    for (int i = 0; i < format.channelCount(); i++)
    {
        int size = pcmBuffer.bytesDataSize();
        uint8_t *data = new uint8_t[size];
        memcpy(data, pcmBuffer.channelData[i], size);
        channelData[i] = data;
    }
}

FAudioPCMBuffer &FAudioPCMBuffer::operator=(const FAudioPCMBuffer &pcmBuffer)
{
    if (this != &pcmBuffer)
    {
        if (channelData)
        {
            for (int i = 0; i < _format.channelCount(); i++)
            {
                if (channelData[i])
                {
                    delete[] channelData[i];
                }
            }
            delete[] channelData;
            channelData = nullptr;
        }

        const QAudioFormat format = pcmBuffer.audioFormat();
        channelData = new uint8_t *[format.channelCount()];
        for (int i = 0; i < format.channelCount(); i++)
        {
            int size = pcmBuffer.bytesDataSize();
            uint8_t *_buffer = new uint8_t[size];
            memcpy(_buffer, pcmBuffer.channelData[i], size);
            channelData[i] = _buffer;
        }
    }
    return *this;
}

FAudioPCMBuffer::~FAudioPCMBuffer()
{
    if (channelData)
    {
        for (int i = 0; i < _format.channelCount(); i++)
        {
            if (channelData[i])
            {
                delete[] channelData[i];
            }
        }
        delete[] channelData;
        channelData = nullptr;
    }
}

QAudioFormat FAudioPCMBuffer::audioFormat() const
{
    return _format;
}

int FAudioPCMBuffer::capacity() const
{
    return _capacity;
}

uint FAudioPCMBuffer::bytesPerSample() const
{
    int size = _format.bytesPerFrame() / _format.channelCount();
    return size;
}

const float **FAudioPCMBuffer::floatChannelData() const
{
    assert(_format.sampleType() == QAudioFormat::SampleType::Float);
    return (const float **)channelData;
}

const int16_t **FAudioPCMBuffer::int16ChannelData() const
{
    assert(_format.sampleType() == QAudioFormat::SampleType::SignedInt);
    return (const int16_t **)channelData;
}

const uint16_t **FAudioPCMBuffer::uint16ChannelData() const
{
    assert(_format.sampleType() == QAudioFormat::SampleType::UnSignedInt);
    return (const uint16_t **)channelData;
}

int FAudioPCMBuffer::bytesDataSize() const
{
    return bytesPerSample() * _capacity;
}

QString FAudioPCMBuffer::debugDescription() const 
{
    QString address;
    address.sprintf("%8p", this);
    return QString("<%1>, start: %2, end: %3, capacity: %4").arg(address).arg(timeRange.start.debugDescription()).arg(timeRange.end.debugDescription()).arg(_capacity);
}
