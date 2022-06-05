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

	public:
		void play();
		void pause();
		void seek(const MediaTime& time);
		void replace(const FVideoDescription* videoDescription);
		MediaTime getCurrentTime() const;
		const PixelBuffer* getPixelBuffer() const;

	public:
		FImageCompositionPipeline* pipeline = nullptr;

	private:
		std::vector<FAsyncImageCompositionRequest> requests;
		mutable std::mutex requestsMutex;
		ks::IRenderEngine* renderEngine = nullptr;

	private:
		PixelBufferPool* pixelBufferPool = nullptr;
		//PixelBuffer* pixelBuffer = nullptr;
		mutable std::mutex requestMutex;
		FAsyncImageCompositionRequest request;
		void flushRequest(const FAsyncImageCompositionRequest& request) const;
		void setCompositionRequest(FAsyncImageCompositionRequest request);
		FAsyncImageCompositionRequest getCompositionRequest() const noexcept;
		//void setPixelBuffer(PixelBuffer* pixelBuffer);
		void resetPixelBufferPool(const FVideoRenderContext& videoRenderContext);

	private:
		MediaTime currentTime = MediaTime::zero;
		MediaTime videoDuration = MediaTime::zero;
		void setCurrentTime(const MediaTime& time);
		void setVideoDuration(const MediaTime& time);
		MediaTime getVideoDuration() const;

	private:
		const int timeScale = 600;
		const unsigned int cacheSize = 30;
		mutable std::mutex gMutex;

		const FVideoDescription* videoDescription = nullptr;
		FVideoRenderContext videoRenderContext;
		SimpleTimer* timer = nullptr;
		Semaphore* semaphore = nullptr;
		MediaTime round(const MediaTime& time, const MediaTime& fps) const;
		void timerTick(const MediaTime& currentTime, const MediaTime& fps);

	private:
		void openRenderImageThread();

	private:
		bool isPause = true;
		mutable std::mutex isPauseMutex;

	private:
		bool isDecodeImageThreadOpen = false;
		bool isDecodeImageThreadWaiting = false;
		mutable std::mutex isDecodeImageThreadWaitingMutex;
		void openDecodeImageThreadIfNeed();
		void openDecodeImageThread();
		void setIsDecodeImageThreadWaiting(const bool flag);
		bool getIsDecodeImageThreadWaiting() const;
	};
}

#endif // !VideoEditor_ImagePlayer_hpp