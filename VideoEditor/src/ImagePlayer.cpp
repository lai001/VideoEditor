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
#include <spdlog/spdlog.h>
#include "Util.hpp"

namespace ks
{
	FImagePlayer::FImagePlayer()
		:renderEngine(ks::FilterContext::renderEngine)
	{
		assert(renderEngine);
		semaphore = new Semaphore(0);
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
		std::lock_guard<std::mutex> isPauseMutexLock(isPauseMutex);
		isPause = false;
	}

	void FImagePlayer::pause()
	{
		std::lock_guard<std::mutex> isPauseMutexLock(isPauseMutex);
		isPause = true;
	}

	void FImagePlayer::seek(const MediaTime & time)
	{
		const MediaTime fps = MediaTime(1.0 / videoDescription->renderContext.videoRenderContext.fps, timeScale);
		const MediaTime seekTime = time;
		if (seekTime == getCurrentTime())
		{
			return;
		}
		setCurrentTime(seekTime);
	}

	void FImagePlayer::replace(const FVideoDescription* videoDescription)
	{
		pause();
		if (timer)
		{
			timer->invalidate();
			timer = nullptr;
		}

		if (videoDescription)
		{
			setVideoDuration(videoDescription->duration());
			this->videoDescription = videoDescription;
			videoRenderContext = videoDescription->renderContext.videoRenderContext;
			resetPixelBufferPool(videoRenderContext);
			openDecodeImageThreadIfNeed();

			const MediaTime fps = MediaTime(1.0 / videoDescription->renderContext.videoRenderContext.fps, timeScale);

			timer = new SimpleTimer(10, [this, fps](SimpleTimer& timer)
			{
				const MediaTime currentTime = getCurrentTime();
				//spdlog::debug("video play time: {:.3f}", currentTime.seconds());

				timerTick(currentTime, fps);

				std::lock_guard<std::mutex> isPauseMutexLock(isPauseMutex);
				if (isPause == false)
				{
					const double seconds = (double)timer.getDuration() / 1000.0;
					MediaTime newTime = currentTime + MediaTime(seconds, 1000000);
					newTime = MediaTimeRange(MediaTime::zero, getVideoDuration()).clamp(newTime);
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

	MediaTime FImagePlayer::getCurrentTime() const
	{
		std::lock_guard<std::mutex> lock(gMutex);
		return currentTime.convertScale(timeScale);
	}

	const PixelBuffer * FImagePlayer::getPixelBuffer() const
	{
		//std::lock_guard<std::mutex> gMutexLock(gMutex);

		PixelBuffer* pixelBuffer = nullptr;

		auto request = getCompositionRequest();
		if (request.getPixelBuffer)
		{
			pixelBuffer = request.getPixelBuffer();
		}

		flushRequest(request);

		return pixelBuffer;
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
				const MediaTime currentTime = round(getCurrentTime(), fps);
				spdlog::info(currentTime.seconds());
				bool isTracing = false;
				bool condition0 = false;
				bool condition1 = false;

				requestsMutex.lock();
				if (requests.empty() == false)
				{
					const FAsyncImageCompositionRequest frontRequest = requests.front();
					condition0 = currentTime < frontRequest.compositionTime;
					condition1 = frontRequest.compositionTime + MediaTime(fps.seconds() * (double)cacheSize, timeScale) < currentTime;
					isTracing = condition0 || condition1;
				}
				else
				{
					if (currentTime < compositionTime)
					{
						isTracing = true;
					}
				}
				requestsMutex.unlock();

				if (isTracing)
				{
					compositionTime = currentTime;
					requestsMutex.lock();
					requests.clear();
					requestsMutex.unlock();

					for (IImageTrack * imageTrack : videoDescription->imageTracks)
					{
						imageTrack->onSeeking(compositionTime);
					}

					//FVideoInstruction videoInstuction;
					//if (videoDescription->videoInstuction(compositionTime, videoInstuction))
					//{
					//	for (IImageTrack *imageTrack : videoInstuction.imageTracks)
					//	{
					//		imageTrack->onSeeking(compositionTime);
					//	}
					//}
				}
				else
				{
					requestsMutex.lock();
					const int requestsSize = requests.size();
					requestsMutex.unlock();
					if (requestsSize >= cacheSize)
					{
						setIsDecodeImageThreadWaiting(true);
						semaphore->wait();
						continue;
					}
				}

				FVideoInstruction videoInstuction;
				FAsyncImageCompositionRequest request;
				request.compositionTime = compositionTime;
				request.videoRenderContext = &videoRenderContext;
				if (videoDescription->videoInstuction(compositionTime, videoInstuction))
				{
					request.instruction = videoInstuction;

					for (IImageTrack *imageTrack : videoInstuction.imageTracks)
					{
						const PixelBuffer *sourceFrame = imageTrack->sourceFrame(compositionTime, videoRenderContext);
						request.sourceFrames[imageTrack->trackID] = sourceFrame;
						//imageTrack->flush(compositionTime);
					}
				}
				else
				{
					continue; // TODO:
				}
				if (pipeline)
				{
					pipeline->composition(request, [this, isTracing]()
					{
						PixelBuffer *newPixelBuffer = pixelBufferPool->pixelBuffer();
						//while (newPixelBuffer == getPixelBuffer())
						//{
						//	newPixelBuffer = pixelBufferPool->pixelBuffer();
						//}
						return newPixelBuffer;
					});
				}

				if (isTracing)
				{
					//this->setPixelBuffer(request.pixelBuffer);
					this->setCompositionRequest(request);
				}

				requestsMutex.lock();
				requests.push_back(request);
				requestsMutex.unlock();

				compositionTime = MediaTime(compositionTime.timeValue() + fps.timeValue(), timeScale);
				compositionTime = MediaTimeRange(MediaTime::zero, getVideoDuration()).clamp(compositionTime);

				if (compositionTime >= getVideoDuration())
				{
					setIsDecodeImageThreadWaiting(true);
					semaphore->wait();
				}
			}
		}).detach();
	}

	void FImagePlayer::setIsDecodeImageThreadWaiting(const bool flag)
	{
		std::lock_guard<std::mutex> lock(isDecodeImageThreadWaitingMutex);
		isDecodeImageThreadWaiting = flag;
	}

	bool FImagePlayer::getIsDecodeImageThreadWaiting() const
	{
		std::lock_guard<std::mutex> lock(isDecodeImageThreadWaitingMutex);
		return isDecodeImageThreadWaiting;
	}

	void FImagePlayer::setCurrentTime(const MediaTime& time)
	{
		std::lock_guard<std::mutex> lock(gMutex);
		currentTime = time.convertScale(timeScale);
	}

	void FImagePlayer::setVideoDuration(const MediaTime& time)
	{
		std::lock_guard<std::mutex> lock(gMutex);
		videoDuration = time.convertScale(timeScale);
	}

	MediaTime FImagePlayer::getVideoDuration() const
	{
		std::lock_guard<std::mutex> lock(gMutex);
		return videoDuration.convertScale(timeScale);
	}

	void FImagePlayer::flushRequest(const FAsyncImageCompositionRequest & request) const
	{
		for (IImageTrack *imageTrack : request.instruction.imageTracks)
		{
			imageTrack->flush(request.compositionTime);
		}
	}

	void FImagePlayer::setCompositionRequest(FAsyncImageCompositionRequest request)
	{
		std::lock_guard<std::mutex> lock(requestMutex);
		this->request = request;
	}

	FAsyncImageCompositionRequest FImagePlayer::getCompositionRequest() const noexcept
	{
		std::lock_guard<std::mutex> lock(requestMutex);
		return request;
	}

	//void FImagePlayer::setPixelBuffer(PixelBuffer * _pixelBuffer)
	//{
	//	std::lock_guard<std::mutex> lock(gMutex);
	//	pixelBuffer = _pixelBuffer;
	//}

	void FImagePlayer::resetPixelBufferPool(const FVideoRenderContext & videoRenderContext)
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
		openDecodeImageThread();
	}

	MediaTime FImagePlayer::round(const MediaTime& time, const MediaTime & fps) const
	{
		assert(fps.seconds() > 0.0);
		assert(time.timeScale() == fps.timeScale());
		int num = (int)floor((time / fps).seconds());
		int timeValue = num * fps.timeValue();
		return MediaTime(timeValue, timeScale);
	}

	void FImagePlayer::timerTick(const MediaTime& currentTime, const MediaTime& fps)
	{
		std::lock_guard<std::mutex> lock(requestsMutex);

		requests.erase(std::remove_if(requests.begin(), requests.end(), [this, fps, currentTime](const FAsyncImageCompositionRequest& request)
		{
			bool flag = request.compositionTime < round(currentTime, fps);
			//if (flag)
			//{
			//	flushRequest(request);
			//}
			return flag;
		}), requests.end());

		if (requests.empty() == false)
		{
			for (auto request = requests.rbegin(); request != requests.rend(); ++request)
			{
				if (request->compositionTime <= currentTime)
				{
					//setPixelBuffer(request->pixelBuffer);
					this->setCompositionRequest(*request);
					break;
				}
			}
		}

		if (requests.empty() == false)
		{
			const FAsyncImageCompositionRequest frontRequest = requests.front();

			if (getIsDecodeImageThreadWaiting() && currentTime < frontRequest.compositionTime)
			{
				//for (size_t i = 0; i < requests.size(); i++)
				//{
				//	flushRequest(requests[i]);
				//}
				requests.clear();
				setIsDecodeImageThreadWaiting(false);
				semaphore->signal();
			}
			else if (getIsDecodeImageThreadWaiting() && requests.size() < cacheSize)
			{
				setIsDecodeImageThreadWaiting(false);
				semaphore->signal();
			}
		}
		else
		{
			if (getIsDecodeImageThreadWaiting())
			{
				setIsDecodeImageThreadWaiting(false);
				semaphore->signal();
			}
		}

	}

	void FImagePlayer::openRenderImageThread()
	{
		// TODO:
	}
}