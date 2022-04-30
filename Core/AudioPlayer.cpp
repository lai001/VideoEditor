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

#include "AudioPlayer.h"
#include "ThirdParty/spdlog.h"

FAudioPlayer::FAudioPlayer(const int samples)
	:samples(samples)
{
	semaphore = new FSemaphore(0);
}

FAudioPlayer::~FAudioPlayer()
{
	if (timer)
	{
		timer->invalidate();
		timer = nullptr;
	}
}

void FAudioPlayer::play()
{
	std::lock_guard<std::mutex> lockg(isPauseMutex);
	isPause = false;
}

void FAudioPlayer::pause()
{
	std::lock_guard<std::mutex> lockg(isPauseMutex);
	isPause = true;
}

void FAudioPlayer::seek(const FMediaTime & time)
{
	const FMediaTime seekTime = time;
	if (seekTime == getCurrentTime())
	{
		return;
	}
	setCurrentTime(seekTime);

	std::lock_guard<std::mutex> lock(buffersMutex);
	buffers.erase(std::remove_if(buffers.begin(), buffers.end(), [&](FAudioCompositionRequest request) {
		if (request.compositionTimeRange.start < time)
		{
			delete request.buffer;
			return true;
		}
		else
		{
			return false;

		}
	}),
		buffers.end());
	const int buffersSize = buffers.size();
	if (buffersSize < cacheSize && getIsDecodeAudioThreadWaiting())
	{
		setIsDecodeAudioThreadWaiting(false);
		semaphore->signal();
	}
}

void FAudioPlayer::replace(const FVideoDescription * videoDescription)
{
	pause();
	if (timer)
	{
		timer->invalidate();
		timer = nullptr;
	}

	if (videoDescription)
	{
		this->videoDescription = videoDescription;
		audioRenderContext = videoDescription->renderContext.audioRenderContext;
		setCurrentTime(FMediaTime(0, audioRenderContext.audioFormat.sampleRate));
		setAudioDuration(videoDescription->duration());
		openDecodeAudioThreadIfNeed();
		timer = new FSimpleTimer(10, [this](FSimpleTimer& timer)
		{
			const FMediaTime currentTime = getCurrentTime();

			std::lock_guard<std::mutex> isPauseMutexLock(isPauseMutex);
			if (isPause == false)
			{
				const double seconds = (double)timer.getDuration() / 1000.0;
				FMediaTime newTime = currentTime + FMediaTime(seconds, 1000000);
				newTime = FMediaTimeRange(FMediaTime::zero, getAudioDuration()).clamp(newTime);
				setCurrentTime(newTime);
			}
		});

		std::lock_guard<std::mutex> isPauseMutexLock(isPauseMutex);
		isPause = false;

		timer->fire();
	}
	else
	{
		assert(false);
		// TODO:
	}
}

FMediaTime FAudioPlayer::getCurrentTime() const
{
	std::lock_guard<std::mutex> lockg(mutex);
	return currentTime.convertScale(audioRenderContext.audioFormat.sampleRate);
}

void FAudioPlayer::getPCMBuffer(std::function<void(const FAudioPCMBuffer*)> retrieval)
{
	std::lock_guard<std::mutex> isPauseMutexLock(isPauseMutex);

	if (isPause)
	{
		retrieval(nullptr);
		return;
	}

	buffersMutex.lock();
	const bool isEmpty = buffers.empty();
	buffersMutex.unlock();

	if (isEmpty)
	{
		retrieval(nullptr);
		return;
	}
	else
	{
		buffersMutex.lock();

		FAudioPCMBuffer *buffer = buffers.begin()->buffer;
		buffers.erase(buffers.begin());
		const int buffersSize = buffers.size();
		buffersMutex.unlock();

		if (buffersSize < cacheSize && getIsDecodeAudioThreadWaiting())
		{
			setIsDecodeAudioThreadWaiting(false);
			semaphore->signal();
		}
		retrieval(buffer);
		delete buffer;
	}
}

void FAudioPlayer::setCurrentTime(const FMediaTime & time)
{
	std::lock_guard<std::mutex> lockg(mutex);
	currentTime = time.convertScale(audioRenderContext.audioFormat.sampleRate);
}

void FAudioPlayer::setAudioDuration(const FMediaTime & time)
{
	std::lock_guard<std::mutex> lockg(mutex);
	audioDuration = time.convertScale(audioRenderContext.audioFormat.sampleRate);
}

FMediaTime FAudioPlayer::getAudioDuration() const
{
	std::lock_guard<std::mutex> lockg(mutex);
	return audioDuration.convertScale(audioRenderContext.audioFormat.sampleRate);
}

void FAudioPlayer::mix(FAudioPCMBuffer & outBuffer, std::vector<FAudioPCMBuffer*> buffers)
{
	bool isNonInterleaved = Bitmask::isContains(outBuffer.audioFormat().formatFlags, AudioFormatFlag::isNonInterleaved);
	bool isFloat = Bitmask::isContains(outBuffer.audioFormat().formatFlags, AudioFormatFlag::isFloat);

	assert(isFloat);
	for (const FAudioPCMBuffer* buffer : buffers)
	{
		assert(outBuffer.audioFormat() == buffer->audioFormat());
	}

	for (FAudioPCMBuffer* buffer : buffers)
	{
		if (isNonInterleaved)
		{
			for (int i = 0; i < buffer->audioFormat().channelsPerFrame; i++)
			{
				float* dst = outBuffer.floatChannelData()[i];
				float* src = buffer->floatChannelData()[i];

				for (int j = 0; j < samples; j++)
				{
					dst[j] += src[j] / (float)buffers.size();
				}
			}
		}
		else
		{
			float* dst = outBuffer.floatChannelData()[0];
			float* src = buffer->floatChannelData()[0];

			for (int j = 0; j < samples * buffer->audioFormat().channelsPerFrame; j++)
			{
				dst[j] += src[j] / (float)buffers.size();
			}
		}
	}
}


FMediaTime FAudioPlayer::round(const FMediaTime & time, const FMediaTime & duration) const
{
	assert(duration.seconds() > 0.0);
	assert(time.timeScale() == duration.timeScale());
	int num = time.timeValue() / duration.timeValue();// (int)floor((time / duration).seconds());
	int timeValue = num * duration.timeValue();
	return FMediaTime(timeValue, audioRenderContext.audioFormat.sampleRate);
}

void FAudioPlayer::openDecodeAudioThreadIfNeed()
{
	if (isDecodeAudioThreadOpen)
	{
		return;
	}
	openDecodeAudioThread();
}

void FAudioPlayer::openDecodeAudioThread()
{
	assert(videoDescription);

	std::thread([this]()
	{
		FMediaTime compositionTime = FMediaTime(0, (int)audioRenderContext.audioFormat.sampleRate);
		const FMediaTime duration = FMediaTime(samples, (int)audioRenderContext.audioFormat.sampleRate);
		while (true)
		{
			const FMediaTime currentTime = round(getCurrentTime(), duration);

			bool isTracing = false;
			bool condition0 = false;
			bool condition1 = false;

			buffersMutex.lock();
			if (buffers.empty() == false)
			{
				FAudioCompositionRequest frontRequest = buffers.front();
				const FAudioPCMBuffer* buffer = frontRequest.buffer;
				condition0 = currentTime < frontRequest.compositionTimeRange.start;
				condition1 = frontRequest.compositionTimeRange.start + FMediaTime(duration.seconds() * (double)cacheSize, duration.timeScale()) < currentTime;
				isTracing = condition0 || condition1;
			}
			else
			{
				if (currentTime + duration < compositionTime)
				{
					isTracing = true;
				}
			}
			buffersMutex.unlock();


			if (isTracing)
			{
				compositionTime = currentTime;
				buffersMutex.lock();
				for (FAudioCompositionRequest request : buffers)
				{
					delete request.buffer;
				}
				buffers.clear();

				buffersMutex.unlock();

				FVideoInstruction videoInstuction;
				if (videoDescription->videoInstuction(compositionTime, videoInstuction))
				{
					for (FAudioTrack* audioTrack : videoInstuction.audioTracks)
					{
						audioTrack->onSeeking(compositionTime);
					}
				}
			}
			else
			{
				buffersMutex.lock();
				const int buffersSize = buffers.size();
				buffersMutex.unlock();
				if (buffersSize >= cacheSize)
				{
					setIsDecodeAudioThreadWaiting(true);
					semaphore->wait();
					continue;
				}
			}

			FVideoInstruction videoInstuction;
			if (videoDescription->videoInstuction(compositionTime, videoInstuction) && compositionTime < getAudioDuration())
			{
				std::vector<FAudioPCMBuffer*> tmpBuffers;
				for (int i = 0; i < videoInstuction.audioTracks.size(); i++)
				{
					tmpBuffers.push_back(new FAudioPCMBuffer(audioRenderContext.audioFormat, samples));
				}
				defer
				{
					for (FAudioPCMBuffer * buffers : tmpBuffers)
					{
						delete buffers;
					}
					tmpBuffers.clear();
				};

				const FMediaTime start = compositionTime;
				const FMediaTime end = FMediaTime(start.timeValue() + duration.timeValue(), duration.timeScale());
				const FMediaTimeRange timeRange = FMediaTimeRange(start, end);

				for (int i = 0; i < videoInstuction.audioTracks.size(); i++)
				{
					videoInstuction.audioTracks[i]->samples(timeRange, tmpBuffers[i]);
					videoInstuction.audioTracks[i]->flush(compositionTime);
				}
				FAudioCompositionRequest request;
				request.compositionTimeRange = timeRange;
				request.buffer = new FAudioPCMBuffer(audioRenderContext.audioFormat, samples);
				FAudioPCMBuffer* outputBuffer = request.buffer;
				this->mix(*outputBuffer, tmpBuffers);
				buffersMutex.lock();
				buffers.push_back(request);
				buffersMutex.unlock();
			}
			else if (compositionTime >= getAudioDuration())
			{
				setIsDecodeAudioThreadWaiting(true);
				semaphore->wait();
				continue; // TODO:
			}
			compositionTime = FMediaTime(compositionTime.timeValue() + samples, (int)audioRenderContext.audioFormat.sampleRate);
			compositionTime = FMediaTimeRange(FMediaTime(0, (int)audioRenderContext.audioFormat.sampleRate), getAudioDuration()).clamp(compositionTime);
		}
	}).detach();
}

void FAudioPlayer::setIsDecodeAudioThreadWaiting(const bool flag)
{
	std::lock_guard<std::mutex> lock(isDecodeAudioThreadWaitingMutex);
	isDecodeAudioThreadWaiting = flag;
}

bool FAudioPlayer::getIsDecodeAudioThreadWaiting() const
{
	std::lock_guard<std::mutex> lock(isDecodeAudioThreadWaitingMutex);
	return isDecodeAudioThreadWaiting;
}
