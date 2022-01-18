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
#include <algorithm>
#include <unordered_map>

FVideoDescription::FVideoDescription()
{
	FAudioFormat format;
	format.sampleRate = 44100;
	format.channelsPerFrame = 2;
	format.bitsPerChannel = 16;
	format.formatType = AudioFormatIdentifiersType::pcm;
	format.formatFlags = 0;
	renderContext.audioRenderContext.audioFormat = format;
}

FVideoDescription::~FVideoDescription()
{

}

void FVideoDescription::prepare()
{
	_duration = FMediaTime::zero;
	std::vector<FMediaTimeRange> timeRanges;
	for (FImageTrack *imageTrack : imageTracks)
	{
		timeRanges.push_back(imageTrack->timeMapping.target);
	}
	for (FAudioTrack *audioTrack : audioTracks)
	{
		timeRanges.push_back(audioTrack->timeMapping.target);
	}
	std::vector<FMediaTimeRange> instructionTimeRanges = FVideoDescription::instructionTimeRanges(timeRanges);

	removeAllVideoInstuctions();

	for (FMediaTimeRange timeRange : instructionTimeRanges)
	{
		FVideoInstruction videoInstruction;
		videoInstruction.timeRange = timeRange;

		for (FImageTrack *imageTrack : imageTracks)
		{
			if (imageTrack->timeMapping.target.intersection(timeRange).isEmpty() == false)
			{
				imageTrack->prepare(renderContext.videoRenderContext);
				videoInstruction.imageTracks.push_back(imageTrack);
			}
		}

		for (FAudioTrack *audioTrack : audioTracks)
		{
			if (audioTrack->timeMapping.target.intersection(timeRange).isEmpty() == false)
			{
				audioTrack->prepare(renderContext.audioRenderContext);
				videoInstruction.audioTracks.push_back(audioTrack);
			}
		}

		_duration = std::max(timeRange.end, _duration);
		videoInstructions.push_back(videoInstruction);
	}
}

std::vector<FMediaTimeRange> FVideoDescription::instructionTimeRanges(std::vector<FMediaTimeRange> timeRanges)
{
	std::unordered_map<double, FMediaTime> keyTimes;

	for (FMediaTimeRange timeRange : timeRanges)
	{
		keyTimes[timeRange.start.seconds()] = timeRange.start;
		keyTimes[timeRange.end.seconds()] = timeRange.end;
	}
	FMediaTime cursor = FMediaTime::zero;

	std::vector<FMediaTimeRange> _timeRanges;

	while (true)
	{

		std::vector<FMediaTime>::iterator minTime;
		std::vector<FMediaTime> greaterThanCursorTimes;

		for (auto args : keyTimes)
		{
			FMediaTime keyTime = keyTimes[args.first];
			if (keyTime > cursor)
			{
				greaterThanCursorTimes.push_back(keyTime);
			}
		}
		if (greaterThanCursorTimes.empty() == false)
		{
			std::sort(greaterThanCursorTimes.begin(), greaterThanCursorTimes.end());
			minTime = std::min_element(greaterThanCursorTimes.begin(), greaterThanCursorTimes.end());

			FMediaTimeRange range = FMediaTimeRange(cursor, *minTime);
			_timeRanges.push_back(range);
			cursor = *minTime;
		}
		else
		{
			break;
		}
	}
	return _timeRanges;
}

bool FVideoDescription::videoInstuction(const FMediaTime time, FVideoInstruction& outVideoInstruction) const
{
	for (FVideoInstruction videoInstruction : videoInstructions)
	{
		if (videoInstruction.timeRange.containsTime(time))
		{
			outVideoInstruction = videoInstruction;
			return true;
		}
	}
	return false;
}

FMediaTime FVideoDescription::duration() const
{
	return _duration;
}

void FVideoDescription::removeAllVideoInstuctions()
{
	videoInstructions.clear();
}
