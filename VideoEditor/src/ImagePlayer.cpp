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

#include "ImagePlayer.hpp"
#include <KSImage/KSImage.hpp>
#include "Util.hpp"

namespace ks
{
	FImagePlayer::FImagePlayer()
		:renderEngine(ks::FilterContext::renderEngine)
	{
		assert(renderEngine);
		semaphore = new Semaphore(0);
		waitingRenderFinishSemaphore = new Semaphore(0);
	}

	FImagePlayer::~FImagePlayer()
	{
		if (timer)
		{
			timer->invalidate();
			timer = nullptr;
		}
	}

	void FImagePlayer::play()
	{
		std::lock_guard<std::mutex> lock(isPauseMutex);
		isPause = false;
	}

	void FImagePlayer::pause()
	{
		std::lock_guard<std::mutex> lock(isPauseMutex);
		isPause = true;
	}

	void FImagePlayer::seek(const MediaTime & time)
	{
		//const MediaTime fps = MediaTime(1.0 / videoDescription->renderContext.videoRenderContext.fps, timeScale);
		const MediaTime seekTime = time;
		std::lock_guard<std::mutex> lock(currentTimeMutex);
		//if (seekTime == currentTime)
		//{
		//	return;
		//}
		currentTime = seekTime;
	}

	void FImagePlayer::replace(const VideoDescription* videoDescription)
	{
		pause();
		if (timer)
		{
			timer->invalidate();
			timer = nullptr;
		}

		if (videoDescription)
		{
			this->videoDuration = videoDescription->duration();
			this->videoDescription = videoDescription;
			videoRenderContext = videoDescription->renderContext.videoRenderContext;
			resetPixelBufferPool(videoRenderContext);
			openDecodeImageThreadIfNeed();

			const MediaTime fps = MediaTime(1.0 / videoDescription->renderContext.videoRenderContext.fps, timeScale);

			timer = new SimpleTimer(25, [this, fps](SimpleTimer& timer)
			{
				timerTick(timer, fps);
			});

			isPause = false;
			timer->fire();
		}
		else
		{
			assert(false);
			// TODO:
		}
	}

	MediaTime FImagePlayer::getCurrentTime() const
	{
		std::lock_guard<std::mutex> lock(currentTimeMutex);
		return currentTime;
	}

	const PixelBuffer * FImagePlayer::getPixelBuffer() const
	{
		//bool isPause = false;
		//{
		//	std::lock_guard<std::mutex> lock(isPauseMutex);
		//	isPause = this->isPause;
		//}
		//if (isPause && cachePixelBuffer)
		//{
		//	return cachePixelBuffer;
		//}
		std::optional<AsyncImageCompositionRequest> request = std::nullopt;
		PixelBuffer* pixelBuffer = nullptr;
		{
			std::lock_guard<std::mutex> lock(requestMutex);
			request = this->request;
			if (request)
			{
				assert(request->getPixelBuffer);
				pixelBuffer = request->getPixelBuffer();
			}
		}
		if (pixelBuffer)
		{
			cachePixelBuffer = pixelBuffer;
		}
		else
		{
			pixelBuffer = cachePixelBuffer;
		}

		{
			std::lock_guard<std::mutex> lock(isWaitingRenderFinishMutex);
			if (isWaitingRenderFinish)
			{
				isWaitingRenderFinish = false;
				waitingRenderFinishSemaphore->signal();
			}
		}

		return pixelBuffer;
	}

	void FImagePlayer::setPipeline(ImageCompositionPipeline * pipeline)
	{
		std::lock_guard<std::mutex> lock(pipelineMutex);
		this->pipeline = pipeline;
	}

	void FImagePlayer::openDecodeImageThread()
	{
		assert(videoDescription);
		const MediaTime fps = MediaTime(1.0 / videoDescription->renderContext.videoRenderContext.fps, timeScale);

		std::thread([this, fps]()
		{
			MediaTime compositionTime = MediaTime::zero;
			while (true)
			{
				MediaTime currentTime = MediaTime::zero;
				MediaTime currentVideoDuration = MediaTime::zero;
				ImageCompositionPipeline* pipeline = nullptr;
				{
					std::lock_guard<std::mutex> lock(currentTimeMutex);
					currentTime = this->currentTime;
				}
				{
					std::lock_guard<std::mutex> lock(videoDurationMutex);
					currentVideoDuration = videoDuration;
				}
				{
					std::lock_guard<std::mutex> lock(pipelineMutex);
					pipeline = this->pipeline;
				}
				bool isTracing = false;
				bool isRequestsEmpty = false;
				bool isLargeThanCacheSize = false;
				std::optional<AsyncImageCompositionRequest> frontRequest = std::nullopt;
				{
					std::lock_guard<std::mutex> lock(requestsMutex);
					isRequestsEmpty = requests.empty();
					isLargeThanCacheSize = requests.size() >= cacheSize;
					if (isRequestsEmpty == false)
					{
						frontRequest = requests.front();
					}
				}

				if (frontRequest)
				{
					const MediaTimeRange timeRange(frontRequest->compositionTime, 
						frontRequest->compositionTime + MediaTime(fps.seconds() * (double)cacheSize, timeScale));
					isTracing = timeRange.containsTime(currentTime) == false;
				}
				else if (currentTime < compositionTime)
				{
					isTracing = true;
				}

				if (isTracing)
				{
					compositionTime = round(currentTime, fps);
					std::lock_guard<std::mutex> lock(requestsMutex);
					requests.clear();
					std::lock_guard<std::mutex> mutexLock(requestMutex);
					this->request = std::nullopt;
					for (IImageTrack * imageTrack : videoDescription->imageTracks)
					{
						imageTrack->onSeeking(compositionTime);
					}
				}
				std::optional<AsyncImageCompositionRequest> nextRequest = getNextRequest(compositionTime);
				if (nextRequest && pipeline)
				{
					std::lock_guard<std::mutex> lock(requestsMutex);
					pipeline->composition(*nextRequest, [this]()
					{
						return pixelBufferPool->pixelBuffer();
					});
					requests.emplace_back(*nextRequest);
				}
				if (isTracing)
				{
					{
						std::lock_guard<std::mutex> mutexLock(requestMutex);
						this->request = nextRequest;
					}
					{
						std::lock_guard<std::mutex> lock(isWaitingRenderFinishMutex);
						isWaitingRenderFinish = true;
					}
					waitingRenderFinishSemaphore->wait();
				}
				compositionTime = MediaTime(compositionTime.timeValue() + fps.timeValue(), timeScale);
				compositionTime = MediaTimeRange(MediaTime::zero, currentVideoDuration).clamp(compositionTime);
				compositionTime = round(compositionTime, fps);
				if (compositionTime >= currentVideoDuration || isLargeThanCacheSize)
				{
					{
						std::lock_guard<std::mutex> lock(isDecodeImageThreadWaitingMutex);
						this->isDecodeImageThreadWaiting = true;
					}
					semaphore->wait();
				}
			}
		}).detach();
	}

	std::optional<AsyncImageCompositionRequest> FImagePlayer::getNextRequest(const MediaTime & compositionTime)
	{
		VideoInstruction videoInstuction;
		if (videoDescription->videoInstuction(compositionTime, videoInstuction))
		{
			AsyncImageCompositionRequest request;
			request.compositionTime = compositionTime;
			request.videoRenderContext = &videoRenderContext;
			request.instruction = videoInstuction;
			for (IImageTrack *imageTrack : videoInstuction.imageTracks)
			{
				const PixelBuffer *sourceFrame = imageTrack->sourceFrame(compositionTime, videoRenderContext);
				request.sourceFrames[imageTrack->trackID] = sourceFrame;
			}
			return request;
		}
		else
		{
			return std::nullopt;
		}
	}

	MediaTime FImagePlayer::round(const MediaTime & time, const MediaTime & fps) const
	{
		assert(fps.seconds() > 0.0);
		//assert(time.timeScale() == fps.timeScale());
		int num = (int)floor((time.convertScale(fps.timeScale()) / fps).seconds());
		int timeValue = num * fps.timeValue();
		return MediaTime(timeValue, timeScale);
	}

	void FImagePlayer::flushRequest(const AsyncImageCompositionRequest & request) const
	{
		for (IImageTrack *imageTrack : request.instruction.imageTracks)
		{
			imageTrack->flush(request.compositionTime);
		}
	}

	void FImagePlayer::resetPixelBufferPool(const VideoRenderContext & videoRenderContext)
	{
		if (pixelBufferPool)
		{
			delete pixelBufferPool;
		}
		const unsigned int width = videoRenderContext.renderSize.width * videoRenderContext.renderScale;
		const unsigned int height = videoRenderContext.renderSize.height * videoRenderContext.renderScale;
		pixelBufferPool = new PixelBufferPool(width, height, cacheSize + 1, videoRenderContext.format);
	}

	void FImagePlayer::openDecodeImageThreadIfNeed()
	{
		if (isDecodeImageThreadOpen)
		{
			return;
		}
		isDecodeImageThreadOpen = true;
		openDecodeImageThread();
	}

	void FImagePlayer::timerTick(const ks::SimpleTimer& timer, const MediaTime& fps)
	{
		bool isPause = false;
		{
			std::lock_guard<std::mutex> lock(isPauseMutex);
			isPause = this->isPause;
		}	
		MediaTime currentTime = MediaTime::zero;
		{
			std::lock_guard<std::mutex> lock(currentTimeMutex);
			currentTime = this->currentTime;
		}
		bool isDecodeImageThreadWaiting = false;
		{
			std::lock_guard<std::mutex> lock(isDecodeImageThreadWaitingMutex);
			isDecodeImageThreadWaiting = this->isDecodeImageThreadWaiting;
		}
		bool isRequestsEmpty = false;
		bool isLessThanCacheSize = false;
		bool isTracing = false;
		{
			std::lock_guard<std::mutex> lock0(requestsMutex);
			std::lock_guard<std::mutex> lock1(requestMutex);
			const std::vector<ks::AsyncImageCompositionRequest> cpRequests = requests;
			requests.clear();
			const std::optional<ks::AsyncImageCompositionRequest> lastRequest = this->request;
			for (auto requestIter = cpRequests.rbegin(); requestIter != cpRequests.rend(); ++requestIter)
			{
				if (requestIter->compositionTime >= currentTime - fps)
				{
					requests.insert(requests.begin(), *requestIter);
					this->request = *requestIter;
				}
			}
			if (lastRequest && this->request && lastRequest->compositionTime != this->request->compositionTime)
			{
				flushRequest(*lastRequest);
			}
			isRequestsEmpty = requests.empty();
			isLessThanCacheSize = requests.size() < cacheSize;

			if (isRequestsEmpty == false)
			{
				const MediaTimeRange timeRange(requests.front().compositionTime,
					requests.front().compositionTime + MediaTime(fps.seconds() * (double)cacheSize, timeScale));
				isTracing = timeRange.containsTime(currentTime) == false;
			}
		}

		const bool isNeedToWakeup = (isRequestsEmpty || isLessThanCacheSize || isTracing) && isDecodeImageThreadWaiting;
		if (isNeedToWakeup)
		{
			{
				std::lock_guard<std::mutex> lock(isDecodeImageThreadWaitingMutex);
				this->isDecodeImageThreadWaiting = false;
			}
			semaphore->signal();
		}

		if (isPause == false)
		{
			MediaTime videoDuration;
			{
				std::lock_guard<std::mutex> lock(videoDurationMutex);
				videoDuration = this->videoDuration;
			}
			const double seconds = (double)timer.getDuration() / 1000.0;
			{
				std::lock_guard<std::mutex> lock(currentTimeMutex);
				MediaTime newTime = this->currentTime + MediaTime(seconds, 1000000);
				newTime = MediaTimeRange(MediaTime::zero, videoDuration).clamp(newTime);
				this->currentTime = newTime;
			}
		}
	}

	void FImagePlayer::openRenderImageThread()
	{
		// TODO:
	}
}