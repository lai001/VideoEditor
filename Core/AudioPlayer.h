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


#ifndef FAUDIOPLAYER_H
#define FAUDIOPLAYER_H

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
#include "Semaphore.h"
#include "AudioPCMBuffer.h"

struct FAudioCompositionRequest
{
	FMediaTimeRange compositionTimeRange = FMediaTimeRange::zero;
	FAudioPCMBuffer* buffer = nullptr;
};

class FAudioPlayer : public boost::noncopyable
{
public:
	FAudioPlayer();
	~FAudioPlayer();

public:
	void play();
	void pause();
	void seek(const FMediaTime& time);
	void replace(const FVideoDescription* videoDescription);

	FMediaTime getCurrentTime() const;
	void getPCMBuffer(std::function<void(const FAudioPCMBuffer*)> retrieval);

private:
	mutable std::mutex mutex;

	FMediaTime currentTime = FMediaTime::zero;
	FMediaTime audioDuration = FMediaTime::zero;
	void setCurrentTime(const FMediaTime& time);
	void setAudioDuration(const FMediaTime& time);
	FMediaTime getAudioDuration() const;

private:
	std::vector<FAudioCompositionRequest> buffers;
	mutable std::mutex buffersMutex;
	void mix(FAudioPCMBuffer& outBuffer, std::vector<FAudioPCMBuffer*> buffers);

private:
	const int samples = 1024;
	const unsigned int cacheSize = 160;

	const FVideoDescription* videoDescription = nullptr;
	FAudioRenderContext audioRenderContext;
	FSimpleTimer* timer = nullptr;
	FSemaphore* semaphore = nullptr;
	FMediaTime round(const FMediaTime& time, const FMediaTime& duration) const;

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

#endif // FAUDIOPLAYER_H
