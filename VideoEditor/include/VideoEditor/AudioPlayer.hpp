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


#ifndef VideoEditor_AudioPlayer_hpp
#define VideoEditor_AudioPlayer_hpp

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include "VideoDescription.hpp"

namespace ks
{
	struct AudioCompositionRequest
	{
		MediaTimeRange compositionTimeRange = MediaTimeRange::zero;
		AudioPCMBuffer* buffer = nullptr;
	};

	class AudioPlayer : public noncopyable
	{
	public:
		AudioPlayer(const int samples = 1024);
		~AudioPlayer();

	public:
		void play();
		void pause();
		void seek(const MediaTime& time);
		void replace(const VideoDescription* videoDescription);

		MediaTime getCurrentTime() const;
		void getPCMBuffer(std::function<void(const AudioPCMBuffer*)> retrieval);

	private:
		mutable std::mutex mutex;

		MediaTime currentTime = MediaTime::zero;
		MediaTime audioDuration = MediaTime::zero;
		void setCurrentTime(const MediaTime& time);
		void setAudioDuration(const MediaTime& time);
		MediaTime getAudioDuration() const;

	private:
		std::vector<AudioCompositionRequest> buffers;
		mutable std::mutex buffersMutex;
		void mix(AudioPCMBuffer& outBuffer, std::vector<AudioPCMBuffer*> buffers);

	private:
		const int samples = 1024;
		const unsigned int cacheSize = 160;

		const VideoDescription* videoDescription = nullptr;
		AudioRenderContext audioRenderContext;
		SimpleTimer* timer = nullptr;
		Semaphore* semaphore = nullptr;
		MediaTime round(const MediaTime& time, const MediaTime& duration) const;

	private:
		bool isPause = true;
		mutable std::mutex isPauseMutex;

	private:
		bool isDecodeAudioThreadOpen = false;
		bool isDecodeAudioThreadWaiting = false;
		mutable std::mutex isDecodeAudioThreadWaitingMutex;
		void openDecodeAudioThreadIfNeed();
		void openDecodeAudioThread();
		void setIsDecodeAudioThreadWaiting(const bool flag);
		bool getIsDecodeAudioThreadWaiting() const;
	};
}

#endif // VideoEditor_AudioPlayer_hpp