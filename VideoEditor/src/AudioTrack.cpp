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

#include "AudioTrack.hpp"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include "Util.hpp"

namespace ks
{
	FAudioTrack::FAudioTrack()
	{
	}

	FAudioTrack::~FAudioTrack()
	{
		if (decoder)
		{
			delete decoder;
			decoder = nullptr;
		}
		flush();
	}

	void FAudioTrack::prepare(const FAudioRenderContext& renderContext)
	{
		if (decoder)
		{
			delete decoder;
			decoder = nullptr;
		}
		AudioFormat format = renderContext.audioFormat;
		decoder = AudioDecoder::New(filePath, format);
		assert(decoder != nullptr);
		outputAudioFormat = format;
	}

	void FAudioTrack::flush()
	{
		std::lock_guard<std::mutex> lock(decoderMutex);

		for (AudioPCMBufferQueueItem item : bufferQueue)
		{
			delete item.pcmBuffer;
		}
		bufferQueue.clear();
	}

	void FAudioTrack::flush(const MediaTime & time)
	{
		std::lock_guard<std::mutex> lock(decoderMutex);

		bufferQueue.erase(std::remove_if(bufferQueue.begin(), bufferQueue.end(), [&](AudioPCMBufferQueueItem queueItem) {
			if (queueItem.timeRange.end < time)
			{
				delete queueItem.pcmBuffer;
				return true;
			}
			else
			{
				return false;

			}
		}),
			bufferQueue.end());
	}

	void FAudioTrack::onSeeking(const MediaTime& time)
	{
		flush();
		if (decoder)
		{
			std::lock_guard<std::mutex> lock(decoderMutex);
			decoder->seek(time);
		}
	}

	void FAudioTrack::samples(const MediaTimeRange& timeRange, AudioPCMBuffer* outAudioPCMBuffer)
	{
		std::lock_guard<std::mutex> lock(decoderMutex);

		std::function<bool()> decodeNextFrame = [this]()
		{
			MediaTimeRange outTimeRange;
			AudioPCMBuffer* outPcmBuffer = decoder->newFrame(outTimeRange);
			if (outPcmBuffer)
			{
				AudioPCMBufferQueueItem item;
				item.pcmBuffer = outPcmBuffer;
				item.timeRange = outTimeRange;
				bufferQueue.push_back(item);
				return true;
			}
			else
			{
				return false;
			}
		};

		while (true)
		{
			if (bufferQueue.empty())
			{
				if (decodeNextFrame() == false)
				{
					break;
				}
			}
			else
			{
				AudioPCMBufferQueueItem& backItem = bufferQueue.back();
				if (backItem.timeRange.end < timeRange.end)
				{
					if (decodeNextFrame() == false)
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		}

		bool isNonInterleaved = outputAudioFormat.formatFlags.isContains(AudioFormatFlag::isNonInterleaved);

		for (const AudioPCMBufferQueueItem& item : bufferQueue)
		{
			MediaTimeRange intersectionTimeRange = item.timeRange.intersection(timeRange);
			int copyNum = intersectionTimeRange.end.timeValue() - intersectionTimeRange.start.timeValue();

			if (intersectionTimeRange.isEmpty())
			{
				continue;
			}

			if (isNonInterleaved)
			{
				int dstOffset = (intersectionTimeRange.start - timeRange.start).timeValue();
				int srcOffset = (intersectionTimeRange.start - item.timeRange.start).timeValue();
				for (int i = 0; i < outputAudioFormat.channelsPerFrame; i++)
				{
					memcpy(outAudioPCMBuffer->channelData()[i] + dstOffset * outputAudioFormat.bytesPerFrame,
						item.pcmBuffer->channelData()[i] + srcOffset * outputAudioFormat.bytesPerFrame,
						outputAudioFormat.bytesPerFrame * copyNum);
				}
			}
			else
			{
				int dstOffset = (intersectionTimeRange.start - timeRange.start).timeValue();
				int srcOffset = (intersectionTimeRange.start - item.timeRange.start).timeValue();
				memcpy(outAudioPCMBuffer->channelData()[0] + dstOffset * outputAudioFormat.bytesPerFrame,
					item.pcmBuffer->channelData()[0] + srcOffset * outputAudioFormat.bytesPerFrame,
					outputAudioFormat.bytesPerFrame * copyNum);
			}

		}
	}
}