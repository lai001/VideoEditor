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

#include "VideoTrack.hpp"
#include <algorithm>
#include "Util.hpp"

namespace ks
{
	VideoTrack::VideoTrack()
	{
	}

	VideoTrack::~VideoTrack()
	{
		if (decoder)
		{
			delete decoder;
		}
		flush();
	}

	const PixelBuffer *VideoTrack::sourceFrame(const MediaTime & compositionTime, const VideoRenderContext & renderContext)
	{
		std::lock_guard<std::mutex> lock(decoderMutex);


		if (decoder == nullptr)
		{
			return nullptr;
		}

		std::function<bool()> decodeNextFrame = [this]()
		{
			MediaTime pts;
			PixelBuffer* pixelBuffer = decoder->newFrame(pts);
			if (pixelBuffer)
			{
				SourceFrame sourceFrame;
				sourceFrame.displayTime = getTargetTime(timeMapping, pts);
				sourceFrame.sourceFrame = pixelBuffer;
				videoFrameQueue.push_back(sourceFrame);
				return true;
			}
			else
			{
				return false;
			}
		};

		while (true)
		{
			if (videoFrameQueue.empty())
			{
				if (decodeNextFrame() == false)
				{
					break;
				}
			}
			else
			{
				SourceFrame back = videoFrameQueue.back();
				if (back.displayTime >= compositionTime)
				{
					break;
				}
				else
				{
					if (decodeNextFrame() == false)
					{
						break;
					}
				}
			}
		}

		if (videoFrameQueue.empty())
		{
			return nullptr;
		}
		else
		{
			SourceFrame back = videoFrameQueue.back();
			return back.sourceFrame;
		}
	}

	const PixelBuffer * VideoTrack::compositionImage(const PixelBuffer & sourceFrame, 
		const MediaTime & compositionTime, 
		const VideoRenderContext & renderContext)
	{
		return &sourceFrame;
	}

	void VideoTrack::prepare(const VideoRenderContext & renderContext)
	{
		if (decoder)
		{
			delete decoder;
		}
		decoder = VideoDecoder::New(filePath, renderContext.format);
		assert(decoder);
		flush();
		onSeeking(timeMapping.source.start);
	}

	void VideoTrack::onSeeking(const MediaTime & compositionTime)
	{
		flush();
		const MediaTime seekTime = getSourceTime(timeMapping, compositionTime);
		std::lock_guard<std::mutex> lock(decoderMutex);

		if (decoder)
		{
			bool seekRet = decoder->seek(seekTime);
		}
	}

	void VideoTrack::flush(const MediaTime & time)
	{
		std::lock_guard<std::mutex> lock(decoderMutex);

		videoFrameQueue.erase(std::remove_if(videoFrameQueue.begin(), videoFrameQueue.end(), [&](SourceFrame videoFrame) {
			if (videoFrame.displayTime < time)
			{
				delete videoFrame.sourceFrame;
				return true;
			}
			else
			{
				return false;

			}
		}),
			videoFrameQueue.end());
	}

	void VideoTrack::flush()
	{
		std::lock_guard<std::mutex> lock(decoderMutex);

		for (auto item : videoFrameQueue)
		{
			delete item.sourceFrame;
		}
		videoFrameQueue.clear();
	}
}