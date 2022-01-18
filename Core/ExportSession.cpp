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

#include "ExportSession.h"

#include <functional>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ThirdParty/FFmpeg.h"
#include "ThirdParty/spdlog.h"

#include "Time/FTime.h"
#include "Utility/FUtility.h"
#include "Resolution.h"
#include "AudioPCMBuffer.h"
#include "PixelBufferPool.h"

FExportSession::FExportSession(const FVideoDescription& videoDescription, FImageCompositionPipeline& imageCompositionPipeline)
	: videoDescription(&videoDescription), imageCompositionPipeline(&imageCompositionPipeline)
{

}

FExportSession::~FExportSession()
{

}

void FExportSession::start(const std::string& filename, std::function<void(const std::string& type, const FMediaTime& time)> progressCallback)
{
	bool isVideoEnable = true;
	bool isAudioEnable = true;

	const FVideoRenderContext videoRenderContext = videoDescription->renderContext.videoRenderContext;
	const FAudioRenderContext audioRenderContext = videoDescription->renderContext.audioRenderContext;

	const FSize renderSize = videoRenderContext.renderSize;
	const float renderScale = videoRenderContext.renderScale;
	float fps = videoRenderContext.fps;
	float audioSampleRate = audioRenderContext.audioFormat.sampleRate;

	assert(audioRenderContext.audioFormat.isNonInterleaved());

	const AVOutputFormat *outputFormat;
	AVFormatContext *outputFormatContext;

	const AVCodec *videoCodec;
	AVStream *videoStream;
	AVCodecContext *videoCodecContext;
	struct SwsContext *videoSwsContext;

	const AVCodec *audioCodec;
	AVStream *audioStream;
	AVCodecContext *audioCodecContext;
	struct SwrContext *audioSwrContext;

	avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, filename.c_str());
	outputFormat = outputFormatContext->oformat;

	videoCodec = avcodec_find_encoder(outputFormat->video_codec);
	videoStream = avformat_new_stream(outputFormatContext, videoCodec);
	videoCodecContext = avcodec_alloc_context3(videoCodec);

	audioCodec = avcodec_find_encoder(outputFormat->audio_codec);
	audioStream = avformat_new_stream(outputFormatContext, audioCodec);
	audioCodecContext = avcodec_alloc_context3(audioCodec);

	videoCodecContext->bit_rate = 40000000;
	videoCodecContext->width = renderSize.width * renderScale;
	videoCodecContext->height = renderSize.height * renderScale;
	videoCodecContext->framerate = FMediaTime(fps, 600).getRational();
	videoCodecContext->time_base = FMediaTime(fps, 600).invert().getRational();
	videoStream->time_base = videoCodecContext->time_base;
	videoCodecContext->gop_size = 1;
	videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
		videoCodecContext->max_b_frames = 2;
	}
	if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
		videoCodecContext->mb_decision = 2;
	}
	if (outputFormat->flags & AVFMT_GLOBALHEADER)
	{
		videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	const AVSampleFormat targetSampleFormat = FUtil::getAVSampleFormat(audioRenderContext.audioFormat);

	audioCodecContext->sample_fmt = targetSampleFormat;//audioCodec->sample_fmts ? audioCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	audioCodecContext->bit_rate = 64000;
	audioCodecContext->sample_rate = audioSampleRate;
	audioCodecContext->channel_layout = av_get_default_channel_layout(audioRenderContext.audioFormat.channelsPerFrame);//AV_CH_LAYOUT_STEREO;
	audioCodecContext->channels = av_get_channel_layout_nb_channels(audioCodecContext->channel_layout);
	audioCodecContext->time_base = FMediaTime(1, audioCodecContext->sample_rate).getRational();
	audioStream->time_base = audioCodecContext->time_base;
	if (outputFormat->flags & AVFMT_GLOBALHEADER)
	{
		audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	avcodec_open2(videoCodecContext, videoCodec, nullptr);
	avcodec_parameters_from_context(videoStream->codecpar, videoCodecContext);

	avcodec_open2(audioCodecContext, audioCodec, nullptr);
	avcodec_parameters_from_context(audioStream->codecpar, audioCodecContext);

	audioSwrContext = swr_alloc_set_opts(nullptr,
		audioCodecContext->channel_layout, targetSampleFormat, audioSampleRate,
		audioCodecContext->channel_layout, targetSampleFormat, audioSampleRate,
		0, nullptr);
	swr_init(audioSwrContext);

	videoSwsContext = sws_getContext(videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_RGBA,
		videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
		SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

	//    av_dump_format(outputFormatContext, 0, filename.toStdString().c_str(), 1);

	if (!(outputFormat->flags & AVFMT_NOFILE)) {
		avio_open(&outputFormatContext->pb, filename.c_str(), AVIO_FLAG_WRITE);
	}
	avformat_write_header(outputFormatContext, nullptr);

	std::function<int(AVFrame *, AVCodecContext *, AVStream *)> encodeFrame = [outputFormatContext](AVFrame *frame, AVCodecContext *codecContext, AVStream *steam)
	{
		int ret = 0;
		ret = avcodec_send_frame(codecContext, frame);
		if (ret < 0)
		{
			spdlog::error(FUtil::ffmpegErrorDescription(ret));
			assert(false);
		}

		while (ret >= 0)
		{
			AVPacket pkt = { 0 };
			ret = avcodec_receive_packet(codecContext, &pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				break;
			}
			else if (ret < 0)
			{
				assert(false);
			}

			av_packet_rescale_ts(&pkt, codecContext->time_base, steam->time_base);

			pkt.stream_index = steam->index;
			av_interleaved_write_frame(outputFormatContext, &pkt);
			av_packet_unref(&pkt);
		}
		return ret;
	};

	{
		AVFrame *frame = av_frame_alloc();
		frame->format = videoCodecContext->pix_fmt;
		frame->width = videoCodecContext->width;
		frame->height = videoCodecContext->height;
		av_frame_get_buffer(frame, 0);
		FMediaTime decodeImageTime = FMediaTime(0, videoCodecContext->time_base.den);

		const unsigned int poolWidth = renderSize.width * renderScale;
		const unsigned int poolHeight = renderSize.height * renderScale;

		FPixelBufferPool* pixelBufferPool = new FPixelBufferPool(poolWidth, poolHeight, 5, videoRenderContext.format);
		defer
		{
			delete pixelBufferPool;
		};

		while (isVideoEnable)
		{
			if (decodeImageTime.seconds() >= videoDescription->duration().seconds())
			{
				break;
			}
			progressCallback("video", decodeImageTime);
			FVideoInstruction videoInstuction;
			if (videoDescription->videoInstuction(decodeImageTime, videoInstuction) == false)
			{
				break;
			}
			defer
			{
				decodeImageTime = decodeImageTime + FMediaTime(1.0 / fps, videoCodecContext->time_base.den);
				decodeImageTime = decodeImageTime.convertScale(videoCodecContext->time_base.den);
			};

			FAsyncImageCompositionRequest request;
			request.instruction = videoInstuction;
			request.videoRenderContext = &videoRenderContext;

			for (FImageTrack *imageTrack : videoInstuction.imageTracks)
			{
				imageTrack->flush(decodeImageTime);
				const FPixelBuffer *sourceFrame = imageTrack->sourceFrame(decodeImageTime, videoDescription->renderContext.videoRenderContext);
				request.sourceFrames[imageTrack->trackID] = sourceFrame;
			}

			imageCompositionPipeline->composition(request, [pixelBufferPool]()
			{
				return pixelBufferPool->pixelBuffer();
			});

			int rgblinesizes[4];
			av_image_fill_linesizes(rgblinesizes, AV_PIX_FMT_RGBA, request.pixelBuffer->width());

			sws_scale(videoSwsContext, &request.pixelBuffer->data()[0],
				rgblinesizes, 0, request.pixelBuffer->height(),
				frame->data, frame->linesize);

			frame->pts = decodeImageTime.timeValue();

			encodeFrame(frame, videoCodecContext, videoStream);
		}

		encodeFrame(nullptr, videoCodecContext, videoStream);

		av_frame_unref(frame);
		av_frame_free(&frame);
	}

	{
		AVFrame *frame = av_frame_alloc();
		frame->format = audioCodecContext->sample_fmt;
		frame->channel_layout = audioCodecContext->channel_layout;
		frame->sample_rate = audioCodecContext->sample_rate;
		frame->nb_samples = audioCodecContext->frame_size == 0 ? 1024 : audioCodecContext->frame_size;
		frame->pts = 0;
		av_frame_get_buffer(frame, 0);
		int ret;
		FMediaTime decodeAudioTime = FMediaTime(0, audioSampleRate);

		FAudioPCMBuffer* outputBuffer = new FAudioPCMBuffer(audioRenderContext.audioFormat, frame->nb_samples);

		defer
		{
			delete outputBuffer;
		};

		while (isAudioEnable)
		{
			if (decodeAudioTime.seconds() >= videoDescription->duration().seconds())
			{
				break;
			}
			progressCallback("audio", decodeAudioTime);

			if (outputBuffer->audioFormat().isNonInterleaved())
			{
				for (int i = 0; i < outputBuffer->audioFormat().channelsPerFrame; i++)
				{
					memset(outputBuffer->channelData()[i], 0, outputBuffer->bytesDataSizePerChannel());
				}
			}
			else
			{
				memset(outputBuffer->channelData()[0], 0, outputBuffer->bytesDataSizePerChannel());
			}

			const FMediaTime duration = FMediaTime(frame->nb_samples, videoDescription->renderContext.audioRenderContext.audioFormat.sampleRate);
			FVideoInstruction videoInstuction;
			if (videoDescription->videoInstuction(decodeAudioTime, videoInstuction) == false)
			{
				break;
			}

			FAudioPCMBuffer* buffer = new FAudioPCMBuffer(audioRenderContext.audioFormat, frame->nb_samples);
			defer
			{
				delete buffer;
			};
			FMediaTimeRange timeRange = FMediaTimeRange(decodeAudioTime, FMediaTime(decodeAudioTime.timeValue() + duration.timeValue(), audioSampleRate));
			for (FAudioTrack* audioTrack : videoInstuction.audioTracks)
			{
				audioTrack->flush(decodeAudioTime);
				bool isNonInterleaved = Bitmask::isContains(buffer->audioFormat().formatFlags, AudioFormatFlag::isNonInterleaved);

				if (isNonInterleaved)
				{
					for (int i = 0; i < buffer->audioFormat().channelsPerFrame; i++)
					{
						memset(buffer->channelData()[i], 0, buffer->bytesDataSizePerChannel());
					}
				}
				else
				{
					memset(buffer->channelData()[0], 0, buffer->bytesDataSizePerChannel());
				}

				audioTrack->samples(timeRange, buffer);

				if (isNonInterleaved)
				{
					for (int i = 0; i < buffer->audioFormat().channelsPerFrame; i++)
					{
						float* dst = outputBuffer->floatChannelData()[i];
						float* src = buffer->floatChannelData()[i];

						for (int j = 0; j < frame->nb_samples; j++)
						{
							dst[j] += src[j] / videoInstuction.audioTracks.size();
						}
					}
				}
				else
				{
					float* dst = outputBuffer->floatChannelData()[0];
					float* src = buffer->floatChannelData()[0];

					for (int j = 0; j < frame->nb_samples * buffer->audioFormat().channelsPerFrame; j++)
					{
						dst[j] += src[j] / videoInstuction.audioTracks.size();
					}
				}
			}

			unsigned char ** inData = outputBuffer->channelData();
			int convertRet = swr_convert(audioSwrContext,
								frame->data, frame->nb_samples,
								const_cast<const uint8_t**>(inData), frame->nb_samples);
			assert(convertRet >= 0);

			encodeFrame(frame, audioCodecContext, audioStream);
			frame->pts = frame->pts + frame->nb_samples;
			decodeAudioTime = FMediaTime(decodeAudioTime.timeValue() + duration.timeValue(), audioSampleRate);
		}
		encodeFrame(nullptr, audioCodecContext, audioStream);
		av_frame_unref(frame);
		av_frame_free(&frame);
	}

	av_write_trailer(outputFormatContext);

	swr_close(audioSwrContext);
	avcodec_free_context(&videoCodecContext);
	avcodec_free_context(&audioCodecContext);
	sws_freeContext(videoSwsContext);
	swr_free(&audioSwrContext);

	if (!(outputFormat->flags & AVFMT_NOFILE))
		avio_closep(&outputFormatContext->pb);

	avformat_free_context(outputFormatContext);

}
