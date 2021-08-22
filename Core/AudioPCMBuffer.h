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

#ifndef AUDIOPCMBUFFER_H
#define AUDIOPCMBUFFER_H

#include "FTime.h"
#include <QAudioFormat>

class FAudioPCMBuffer
{
private:
    QAudioFormat _format;
    int _capacity;
    uint8_t** channelData = nullptr;

public:
    FAudioPCMBuffer(const QAudioFormat format, const int capacity);
    FAudioPCMBuffer(const FAudioPCMBuffer& buffer);
    FAudioPCMBuffer &operator=(const FAudioPCMBuffer &buffer);
    ~FAudioPCMBuffer();

    FMediaTimeRange timeRange;

    QAudioFormat audioFormat() const;
    int capacity() const;
    uint bytesPerSample() const;
    int bytesDataSize() const;

    const float **floatChannelData() const;
    const int16_t **int16ChannelData() const;
    const uint16_t **uint16ChannelData() const;

    QString debugDescription() const;
};

#endif // AUDIOPCMBUFFER_H