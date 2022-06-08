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

#ifndef VideoEditor_ImagePlayer_hpp
#define VideoEditor_ImagePlayer_hpp

#include <vector>
#include <optional>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <KSRenderEngine/KSRenderEngine.hpp>
#include "VideoDescription.hpp"
#include "ImageCompositionPipeline.hpp"

namespace ks
{
	class FImagePlayer : public noncopyable
	{
	public:
		FImagePlayer();
		~FImagePlayer();
		void play();
		void pause();
		void seek(const MediaTime& time);
		void replace(const VideoDescription* videoDescription);
		MediaTime getCurrentTime() const;
		const PixelBuffer* getPixelBuffer() const;
		void setPipeline(ImageCompositionPipeline* pipeline);

	private:
		mutable std::mutex currentTimeMutex;
		mutable std::mutex isPauseMutex;
		mutable std::mutex requestMutex;
		mutable std::mutex requestsMutex;
		mutable std::mutex pipelineMutex;
		mutable std::mutex videoDurationMutex;
		mutable std::mutex isDecodeImageThreadWaitingMutex;
		mutable std::mutex isWaitingRenderFinishMutex;

		mutable ks::PixelBuffer* cachePixelBuffer = nullptr;
		ImageCompositionPipeline* pipeline = nullptr;
		std::vector<AsyncImageCompositionRequest> requests;
		ks::IRenderEngine* renderEngine = nullptr;
		PixelBufferPool* pixelBufferPool = nullptr;
		MediaTime currentTime = MediaTime::zero;
		MediaTime videoDuration = MediaTime::zero;
		const int timeScale = 600;
		const unsigned int cacheSize = 30;
		const VideoDescription* videoDescription = nullptr;
		VideoRenderContext videoRenderContext;
		SimpleTimer* timer = nullptr;
		Semaphore* semaphore = nullptr;
		Semaphore* waitingRenderFinishSemaphore = nullptr;
		std::optional<AsyncImageCompositionRequest> request = std::nullopt;
		bool isPause = true;
		bool isDecodeImageThreadOpen = false;
		bool isDecodeImageThreadWaiting = false;
		mutable bool isWaitingRenderFinish = false;

		void flushRequest(const AsyncImageCompositionRequest& request) const;
		void resetPixelBufferPool(const VideoRenderContext& videoRenderContext);
		void timerTick(const ks::SimpleTimer& timer, const MediaTime& fps);
		void openRenderImageThread();
		void openDecodeImageThreadIfNeed();
		void openDecodeImageThread();
		std::optional<AsyncImageCompositionRequest> getNextRequest(const MediaTime& compositionTime);
		MediaTime round(const MediaTime& time, const MediaTime & fps) const;
	};
}

#endif // !VideoEditor_ImagePlayer_hpp