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

#ifndef VideoEditor_AudioTrack_hpp
#define VideoEditor_AudioTrack_hpp

#include <mutex> 
#include <vector>
#include <string>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include "RenderContext.hpp"
#include "MediaTrack.hpp"

namespace ks
{
	struct AudioPCMBufferQueueItem
	{
		MediaTimeRange timeRange;
		AudioPCMBuffer* pcmBuffer = nullptr;
	};

	class FAudioTrack : public IMediaTrack
	{
	public:
		FAudioTrack();
		~FAudioTrack();

	public:
		std::string filePath;

	public:
		virtual void prepare(const AudioRenderContext& renderContext);
		virtual void flush();
		virtual void flush(const MediaTime& time);
		virtual void onSeeking(const MediaTime& time);
		virtual void samples(const MediaTimeRange& timeRange, AudioPCMBuffer* outAudioPCMBuffer);

	private:
		std::vector<AudioPCMBufferQueueItem> bufferQueue;
		AudioDecoder* decoder = nullptr;
		AudioFormat outputAudioFormat;
		MediaTime time;
		std::mutex decoderMutex;
	};
}

#endif // VideoEditor_AudioTrack_hpp