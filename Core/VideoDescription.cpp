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

#include "VideoDescription.h"
#include <QMap>

FVideoDescription::FVideoDescription()
{
    int sampleRate = 44100;
    QAudioFormat format;
    format.setSampleRate(sampleRate);                
    format.setChannelCount(2);                       
    format.setSampleSize(16);                       
    format.setCodec("audio/pcm");                   
    format.setByteOrder(QAudioFormat::LittleEndian); 
    format.setSampleType(QAudioFormat::UnSignedInt); 
    audioFormat = format;
}

FVideoDescription::~FVideoDescription()
{
    removeAllVideoInstuctions();
}

void FVideoDescription::prepare()
{
    _duration = FMediaTime::zero;
    QVector<FMediaTimeRange> timeRanges;
    for (FImageTrack *imageTrack : imageTracks)
    {
        timeRanges.append(imageTrack->timeMapping.target);
    }
    for (FAudioTrack *audioTrack : audioTracks)
    {
        timeRanges.append(audioTrack->timeMapping.target);
    }
    QVector<FMediaTimeRange> instructionTimeRanges = FVideoDescription::instructionTimeRanges(timeRanges);

    removeAllVideoInstuctions();

    for (FMediaTimeRange timeRange : instructionTimeRanges)
    {
        FVideoInstruction *videoInstruction = new FVideoInstruction();
        videoInstruction->timeRange = timeRange;

        for (FImageTrack *imageTrack : imageTracks)
        {
            if (imageTrack->timeMapping.target.intersection(timeRange).isEmpty() == false)
            {
                imageTrack->prepare(this);
                videoInstruction->imageTracks.append(imageTrack);
            }
        }

        for (FAudioTrack *audioTrack : audioTracks)
        {
            if (audioTrack->timeMapping.target.intersection(timeRange).isEmpty() == false)
            {
                audioTrack->prepare(this);
                videoInstruction->audioTracks.append(audioTrack);
            }
        }

        _duration = qMax(timeRange.end, _duration);
        videoInstructions.append(videoInstruction);
    }
}

QVector<FMediaTimeRange> FVideoDescription::instructionTimeRanges(QVector<FMediaTimeRange> timeRanges)
{
    QMap<double, FMediaTime> keyTimes;

    for (FMediaTimeRange timeRange : timeRanges)
    {
        keyTimes[timeRange.start.seconds()] = timeRange.start;
        keyTimes[timeRange.end.seconds()] = timeRange.end;
    }
    FMediaTime cursor = FMediaTime::zero;

    QVector<FMediaTimeRange> _timeRanges;

    while (true)
    {
        FMediaTime *minTime = nullptr;
        QVector<FMediaTime> greaterThanCursorTimes;
        for (auto key : keyTimes.keys())
        {
            FMediaTime keyTime = keyTimes[key];
            if (keyTime > cursor)
            {
                greaterThanCursorTimes.append(keyTime);
            }
        }
        if (greaterThanCursorTimes.isEmpty() == false)
        {
            qSort(greaterThanCursorTimes.begin(), greaterThanCursorTimes.end());
            minTime = std::min_element(greaterThanCursorTimes.begin(), greaterThanCursorTimes.end());
            if (minTime == nullptr)
            {
                break;
            }
            else
            {
                FMediaTimeRange range = FMediaTimeRange(cursor, *minTime);
                _timeRanges.append(range);
                cursor = *minTime;
            }
        }
        else
        {
            break;
        }
    }
    return _timeRanges;
}

FVideoInstruction *FVideoDescription::videoInstuction(const FMediaTime time)
{
    FVideoInstruction *mvideoInstruction = nullptr;
    for (FVideoInstruction *videoInstruction : videoInstructions)
    {
        if (videoInstruction->timeRange.containsTime(time))
        {
            mvideoInstruction = videoInstruction;
            break;
        }
    }
    return mvideoInstruction;
}

FMediaTime FVideoDescription::duration() const
{
    return _duration;
}

void FVideoDescription::removeAllVideoInstuctions()
{
    for (FVideoInstruction *videoInstruction : videoInstructions)
    {
        delete videoInstruction;
    }
    videoInstructions.clear();
}
