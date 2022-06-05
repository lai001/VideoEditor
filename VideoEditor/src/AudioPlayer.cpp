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

#include "AudioPlayer.hpp"

namespace ks
{
	FAudioPlayer::FAudioPlayer(const int samples)
		:samples(samples)
	{
		semaphore = new Semaphore(0);
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

	void FAudioPlayer::seek(const MediaTime & time)
	{
		const MediaTime seekTime = time;
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
			setCurrentTime(MediaTime(0, audioRenderContext.audioFormat.sampleRate));
			setAudioDuration(videoDescription->duration());
			openDecodeAudioThreadIfNeed();
			timer = new SimpleTimer(10, [this](SimpleTimer& timer)
			{
				const MediaTime currentTime = getCurrentTime();

				std::lock_guard<std::mutex> isPauseMutexLock(isPauseMutex);
				if (isPause == false)
				{
					const double seconds = (double)timer.getDuration() / 1000.0;
					MediaTime newTime = currentTime + MediaTime(seconds, 1000000);
					newTime = MediaTimeRange(MediaTime::zero, getAudioDuration()).clamp(newTime);
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

	MediaTime FAudioPlayer::getCurrentTime() const
	{
		std::lock_guard<std::mutex> lockg(mutex);
		return currentTime.convertScale(audioRenderContext.audioFormat.sampleRate);
	}

	void FAudioPlayer::getPCMBuffer(std::function<void(const AudioPCMBuffer*)> retrieval)
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

			AudioPCMBuffer *buffer = buffers.begin()->buffer;
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

	void FAudioPlayer::setCurrentTime(const MediaTime & time)
	{
		std::lock_guard<std::mutex> lockg(mutex);
		currentTime = time.convertScale(audioRenderContext.audioFormat.sampleRate);
	}

	void FAudioPlayer::setAudioDuration(const MediaTime & time)
	{
		std::lock_guard<std::mutex> lockg(mutex);
		audioDuration = time.convertScale(audioRenderContext.audioFormat.sampleRate);
	}

	MediaTime FAudioPlayer::getAudioDuration() const
	{
		std::lock_guard<std::mutex> lockg(mutex);
		return audioDuration.convertScale(audioRenderContext.audioFormat.sampleRate);
	}

	void FAudioPlayer::mix(AudioPCMBuffer & outBuffer, std::vector<AudioPCMBuffer*> buffers)
	{
		bool isNonInterleaved = outBuffer.audioFormat().formatFlags.isContains(AudioFormatFlag::isNonInterleaved);
		bool isFloat = outBuffer.audioFormat().formatFlags.isContains(AudioFormatFlag::isFloat);

		assert(isFloat);
		for (const AudioPCMBuffer* buffer : buffers)
		{
			assert(outBuffer.audioFormat() == buffer->audioFormat());
		}

		for (AudioPCMBuffer* buffer : buffers)
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


	MediaTime FAudioPlayer::round(const MediaTime & time, const MediaTime & duration) const
	{
		assert(duration.seconds() > 0.0);
		assert(time.timeScale() == duration.timeScale());
		int num = time.timeValue() / duration.timeValue();// (int)floor((time / duration).seconds());
		int timeValue = num * duration.timeValue();
		return MediaTime(timeValue, audioRenderContext.audioFormat.sampleRate);
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
			MediaTime compositionTime = MediaTime(0, (int)audioRenderContext.audioFormat.sampleRate);
			const MediaTime duration = MediaTime(samples, (int)audioRenderContext.audioFormat.sampleRate);
			while (true)
			{
				const MediaTime currentTime = round(getCurrentTime(), duration);

				bool isTracing = false;
				bool condition0 = false;
				bool condition1 = false;

				buffersMutex.lock();
				if (buffers.empty() == false)
				{
					FAudioCompositionRequest frontRequest = buffers.front();
					const AudioPCMBuffer* buffer = frontRequest.buffer;
					condition0 = currentTime < frontRequest.compositionTimeRange.start;
					condition1 = frontRequest.compositionTimeRange.start + MediaTime(duration.seconds() * (double)cacheSize, duration.timeScale()) < currentTime;
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
					std::vector<AudioPCMBuffer*> tmpBuffers;
					for (int i = 0; i < videoInstuction.audioTracks.size(); i++)
					{
						tmpBuffers.push_back(new AudioPCMBuffer(audioRenderContext.audioFormat, samples));
					}
					defer
					{
						for (AudioPCMBuffer * buffers : tmpBuffers)
						{
							delete buffers;
						}
						tmpBuffers.clear();
					};

					const MediaTime start = compositionTime;
					const MediaTime end = MediaTime(start.timeValue() + duration.timeValue(), duration.timeScale());
					const MediaTimeRange timeRange = MediaTimeRange(start, end);

					for (int i = 0; i < videoInstuction.audioTracks.size(); i++)
					{
						videoInstuction.audioTracks[i]->samples(timeRange, tmpBuffers[i]);
						videoInstuction.audioTracks[i]->flush(compositionTime);
					}
					FAudioCompositionRequest request;
					request.compositionTimeRange = timeRange;
					request.buffer = new AudioPCMBuffer(audioRenderContext.audioFormat, samples);
					AudioPCMBuffer* outputBuffer = request.buffer;
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
				compositionTime = MediaTime(compositionTime.timeValue() + samples, (int)audioRenderContext.audioFormat.sampleRate);
				compositionTime = MediaTimeRange(MediaTime(0, (int)audioRenderContext.audioFormat.sampleRate), getAudioDuration()).clamp(compositionTime);
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
}