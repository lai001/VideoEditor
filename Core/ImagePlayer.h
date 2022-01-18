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


#ifndef FIMAGEPLAYER_H
#define FIMAGEPLAYER_H

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <future>

#include "Vendor/noncopyable.hpp"
#include "VideoDescription.h"
#include "Time/FTime.h"
#include "SimpleTimer.h"
#include "PixelBuffer.h"
#include "ImageCompositionPipeline.h"
#include "Semaphore.h"
#include "PixelBufferPool.h"

class FImagePlayer : public boost::noncopyable
{
public:
	FImagePlayer();
	~FImagePlayer();

public:
	void play();
	void pause();
	void seek(const FMediaTime& time);
	void replace(const FVideoDescription* videoDescription);
	FMediaTime getCurrentTime() const;
	const FPixelBuffer* getPixelBuffer() const;

public:
	FImageCompositionPipeline* pipeline = nullptr;

private:
	std::vector<FAsyncImageCompositionRequest> requests;
	mutable std::mutex requestsMutex;

private:
	FPixelBufferPool* pixelBufferPool = nullptr;
	FPixelBuffer* pixelBuffer = nullptr;
	void setPixelBuffer(FPixelBuffer* pixelBuffer);
	void resetPixelBufferPool(const FVideoRenderContext& videoRenderContext);

private:
	FMediaTime currentTime = FMediaTime::zero;
	FMediaTime videoDuration = FMediaTime::zero;
	void setCurrentTime(const FMediaTime& time);
	void setVideoDuration(const FMediaTime& time);
	FMediaTime getVideoDuration() const;

private:
	const int timeScale = 600;
	const unsigned int cacheSize = 30;
	mutable std::mutex gMutex;

	const FVideoDescription* videoDescription = nullptr;
	FVideoRenderContext videoRenderContext;
	FSimpleTimer* timer = nullptr;
	FSemaphore* semaphore = nullptr;
	FMediaTime round(const FMediaTime& time, const FMediaTime& fps) const;
	void timerTick(const FMediaTime& currentTime, const FMediaTime& fps);

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

#endif // !FIMAGEPLAYER_H