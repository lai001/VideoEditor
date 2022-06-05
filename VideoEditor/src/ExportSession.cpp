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

#include "ExportSession.hpp"

#include <functional>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <spdlog/spdlog.h>
#include "Util.hpp"
#include "Resolution.hpp"

namespace ks
{
	FExportSession::FExportSession(const FVideoDescription& videoDescription, FImageCompositionPipeline& imageCompositionPipeline)
		: videoDescription(&videoDescription), imageCompositionPipeline(&imageCompositionPipeline)
	{

	}

	FExportSession::~FExportSession()
	{

	}

	void FExportSession::start(const std::string& filename,
		std::function<void(const FExportSession::EncodeType& type, const MediaTime& time)> progressCallback)
	{
		assert(videoDescription);
		assert(imageCompositionPipeline);

		const FVideoRenderContext videoRenderContext = videoDescription->renderContext.videoRenderContext;
		const FAudioRenderContext audioRenderContext = videoDescription->renderContext.audioRenderContext;
		assert(audioRenderContext.audioFormat.isNonInterleaved());
		assert(audioRenderContext.audioFormat.isFloat());

		VideoFileEncoder::VideoEncodeAttribute videoEncodeAttribute;
		videoEncodeAttribute.videoWidth = videoRenderContext.renderSize.width * videoRenderContext.renderScale;
		videoEncodeAttribute.videoHeight = videoRenderContext.renderSize.height * videoRenderContext.renderScale;
		videoEncodeAttribute.fps = MediaTime(1, 24);
		videoEncodeAttribute.timeBase = MediaTime(1, 600);
		videoEncodeAttribute.bitRate = 6 * 1000 * 1000;
		videoEncodeAttribute.gopSize = 15;
		videoEncodeAttribute.pixelBufferFormatType = PixelBuffer::FormatType::yuv420p;

		const AudioFormat audioFormat = audioRenderContext.audioFormat;

		VideoFileEncoder::Error error;
		std::unique_ptr<VideoFileEncoder> videoFileEncoder =
			std::unique_ptr<VideoFileEncoder>(VideoFileEncoder::New(filename, videoEncodeAttribute, audioFormat, &error));
		assert(videoFileEncoder);
		std::unique_ptr<PixelBufferPool> pixelBufferPool = std::make_unique<PixelBufferPool>(videoEncodeAttribute.videoWidth,
			videoEncodeAttribute.videoHeight,
			5,
			videoRenderContext.format);

		MediaTime encodeImageTime = MediaTime(0, videoEncodeAttribute.timeBase.timeValue());

		while (true)
		{
			if (encodeImageTime.seconds() >= videoDescription->duration().seconds())
			{
				break;
			}
			progressCallback(FExportSession::EncodeType::video, encodeImageTime);
			FVideoInstruction videoInstuction;
			if (videoDescription->videoInstuction(encodeImageTime, videoInstuction) == false)
			{
				break;
			}
			defer
			{
				encodeImageTime = encodeImageTime + videoEncodeAttribute.fps;
				encodeImageTime = encodeImageTime.convertScale(videoEncodeAttribute.timeBase.timeScale());
			};

			FAsyncImageCompositionRequest request;
			request.instruction = videoInstuction;
			request.videoRenderContext = &videoRenderContext;

			for (IImageTrack *imageTrack : videoInstuction.imageTracks)
			{
				imageTrack->flush(encodeImageTime);
				const PixelBuffer *sourceFrame = imageTrack->sourceFrame(encodeImageTime, videoDescription->renderContext.videoRenderContext);
				request.sourceFrames[imageTrack->trackID] = sourceFrame;
			}

			imageCompositionPipeline->composition(request, [&pixelBufferPool]()
			{
				return pixelBufferPool->pixelBuffer();
			});
			
			videoFileEncoder->encode(*request.getPixelBuffer(), encodeImageTime);
		}

		MediaTime encodeAudioTime = MediaTime(0, audioFormat.sampleRate);
		std::unique_ptr<AudioPCMBuffer> outputBuffer = std::make_unique<AudioPCMBuffer>(audioRenderContext.audioFormat,
			videoFileEncoder->getAudioSamples());

		while (true)
		{
			if (encodeAudioTime.seconds() >= videoDescription->duration().seconds())
			{
				break;
			}
			progressCallback(FExportSession::EncodeType::audio, encodeAudioTime);

			const MediaTime duration = MediaTime(static_cast<int>(videoFileEncoder->getAudioSamples()), audioFormat.sampleRate);
			FVideoInstruction videoInstuction;
			if (videoDescription->videoInstuction(encodeAudioTime, videoInstuction) == false)
			{
				break;
			}
			outputBuffer->setZero();
			std::unique_ptr<AudioPCMBuffer> buffer = std::make_unique<AudioPCMBuffer>(audioRenderContext.audioFormat,
				videoFileEncoder->getAudioSamples());

			MediaTimeRange timeRange = MediaTimeRange(encodeAudioTime, MediaTime(encodeAudioTime.timeValue() + duration.timeValue(), audioFormat.sampleRate));
			for (FAudioTrack* audioTrack : videoInstuction.audioTracks)
			{
				audioTrack->flush(encodeAudioTime);
				buffer->setZero();
				audioTrack->samples(timeRange, buffer.get());

				if (audioFormat.isNonInterleaved())
				{
					for (int i = 0; i < buffer->audioFormat().channelsPerFrame; i++)
					{
						float* dst = outputBuffer->floatChannelData()[i];
						float* src = buffer->floatChannelData()[i];

						for (int j = 0; j < buffer->frameCapacity(); j++)
						{
							dst[j] += src[j] / videoInstuction.audioTracks.size();
						}
					}
				}
				else
				{
					float* dst = outputBuffer->floatChannelData()[0];
					float* src = buffer->floatChannelData()[0];

					for (int j = 0; j < buffer->frameCapacity() * buffer->audioFormat().channelsPerFrame; j++)
					{
						dst[j] += src[j] / videoInstuction.audioTracks.size();
					}
				}
			}
			videoFileEncoder->encode(*outputBuffer, encodeAudioTime);
			encodeAudioTime = MediaTime(encodeAudioTime.timeValue() + duration.timeValue(), audioFormat.sampleRate);
		}

		videoFileEncoder->encodeTail();

	}
}