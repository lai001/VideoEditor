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

#include "VideoDescription.hpp"
#include <algorithm>
#include <unordered_map>

namespace ks
{
	FVideoDescription::FVideoDescription()
	{
		AudioFormat format;
		format.sampleRate = 44100;
		format.channelsPerFrame = 2;
		format.bitsPerChannel = 16;
		format.formatType = AudioFormatIdentifiersType::pcm;
		format.formatFlags = ks::AudioFormatFlag();
		renderContext.audioRenderContext.audioFormat = format;
	}

	FVideoDescription::~FVideoDescription()
	{

	}

	void FVideoDescription::prepare()
	{
		_duration = MediaTime::zero;
		std::vector<MediaTimeRange> timeRanges;
		for (size_t i = 0; i < imageTracks.size(); i++)
		{
			imageTracks[i]->trackID = i;
			timeRanges.push_back(imageTracks[i]->timeMapping.target);
		}
		for (size_t i = 0; i < audioTracks.size(); i++)
		{
			audioTracks[i]->trackID = i;
			timeRanges.push_back(audioTracks[i]->timeMapping.target);
		}
		std::vector<MediaTimeRange> instructionTimeRanges = FVideoDescription::instructionTimeRanges(timeRanges);

		removeAllVideoInstuctions();

		for (MediaTimeRange timeRange : instructionTimeRanges)
		{
			FVideoInstruction videoInstruction;
			videoInstruction.timeRange = timeRange;

			for (IImageTrack *imageTrack : imageTracks)
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

	std::vector<MediaTimeRange> FVideoDescription::instructionTimeRanges(std::vector<MediaTimeRange> timeRanges)
	{
		std::unordered_map<double, MediaTime> keyTimes;

		for (MediaTimeRange timeRange : timeRanges)
		{
			keyTimes[timeRange.start.seconds()] = timeRange.start;
			keyTimes[timeRange.end.seconds()] = timeRange.end;
		}
		MediaTime cursor = MediaTime::zero;

		std::vector<MediaTimeRange> _timeRanges;

		while (true)
		{

			std::vector<MediaTime>::iterator minTime;
			std::vector<MediaTime> greaterThanCursorTimes;

			for (auto args : keyTimes)
			{
				MediaTime keyTime = keyTimes[args.first];
				if (keyTime > cursor)
				{
					greaterThanCursorTimes.push_back(keyTime);
				}
			}
			if (greaterThanCursorTimes.empty() == false)
			{
				std::sort(greaterThanCursorTimes.begin(), greaterThanCursorTimes.end());
				minTime = std::min_element(greaterThanCursorTimes.begin(), greaterThanCursorTimes.end());

				MediaTimeRange range = MediaTimeRange(cursor, *minTime);
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

	bool FVideoDescription::videoInstuction(const MediaTime time, FVideoInstruction& outVideoInstruction) const
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

	MediaTime FVideoDescription::duration() const
	{
		return _duration;
	}

	void FVideoDescription::removeAllVideoInstuctions()
	{
		videoInstructions.clear();
	}
}