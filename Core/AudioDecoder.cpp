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

#include "AudioDecoder.h"

#include <assert.h>

#include "Utility/FUtility.h"

FAudioDecoder * FAudioDecoder::New(const std::string & filePath, const FAudioFormat & format)
{
	SwrContext *swrctx = nullptr;
	AVFormatContext *formatContext = nullptr;
	AVStream *audioStream = nullptr;
	AVCodecContext *audioCodecCtx = nullptr;
	AVCodec *codec = nullptr;
	int audioStreamIndex = -1;

	do
	{
		AVFormatContext *formatContext = avformat_alloc_context();

		if (avformat_open_input(&formatContext, filePath.c_str(), NULL, NULL) != 0)
		{
			break;
		}

		if (avformat_find_stream_info(formatContext, NULL) < 0)
		{
			break;
		}

		for (unsigned int i = 0; i < formatContext->nb_streams; i++)
		{
			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				audioStreamIndex = i;
				audioStream = formatContext->streams[i];
				break;
			}
		}
		if (audioStreamIndex == -1)
		{
			break;
		}

		codec = avcodec_find_decoder(audioStream->codecpar->codec_id);

		if (codec == nullptr)
		{
			break;
		}

		audioCodecCtx = avcodec_alloc_context3(codec);

		if (audioCodecCtx == nullptr)
		{
			break;
		}

		avcodec_parameters_to_context(audioCodecCtx, audioStream->codecpar);

		if (avcodec_open2(audioCodecCtx, codec, NULL) < 0)
		{
			break;
		}
		AVSampleFormat sampleFormat = FUtil::getAVSampleFormat(format);

		swrctx = swr_alloc_set_opts(swrctx,
			av_get_default_channel_layout(format.channelsPerFrame), sampleFormat, format.sampleRate,
			audioCodecCtx->channel_layout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate,
			0, nullptr);
		if (swrctx == nullptr)
		{
			break;
		}

		if (swr_init(swrctx) < 0)
		{
			break;
		}

		FAudioDecoder* audioDecoder = new FAudioDecoder();
		audioDecoder->filePath = filePath;
		audioDecoder->outputAudioFormat = format;
		audioDecoder->swrctx = swrctx;
		audioDecoder->formatContext = formatContext;
		audioDecoder->audioStream = audioStream;
		audioDecoder->audioCodecCtx = audioCodecCtx;
		audioDecoder->codec = codec;
		audioDecoder->audioStreamIndex = audioStreamIndex;

		return audioDecoder;
	} while (true);

	if (swrctx)
	{
		swr_close(swrctx);
		swr_free(&swrctx);
	}
	if (audioCodecCtx)
	{
		avcodec_close(audioCodecCtx);
		avcodec_free_context(&audioCodecCtx);
	}
	if (formatContext)
	{
		avformat_close_input(&formatContext);
		avformat_free_context(formatContext);
	}

	return nullptr;
}

FAudioDecoder::FAudioDecoder()
{
}

FAudioDecoder::~FAudioDecoder()
{
	swr_close(swrctx);
	swr_free(&swrctx);

	avcodec_close(audioCodecCtx);
	avcodec_free_context(&audioCodecCtx);

	avformat_close_input(&formatContext);
	avformat_free_context(formatContext);
}

std::string FAudioDecoder::getFilePath() const
{
	return filePath;
}

FAudioPCMBuffer* FAudioDecoder::newFrame(FMediaTimeRange& outTimeRange)
{
	AVFrame *frame = av_frame_alloc();
	AVPacket *packet = av_packet_alloc();
	defer
	{
		av_packet_free(&packet);
		av_frame_free(&frame);
	};
	while (true)
	{
		defer
		{
			av_packet_unref(packet);
			av_frame_unref(frame);
		};

		int ret = av_read_frame(formatContext, packet);
		if (ret < 0)
		{
			break;
		}
		else
		{
			if (packet->stream_index == audioStreamIndex)
			{
				FAudioPCMBuffer* buffer = newDecodedPCMBuffer(audioCodecCtx, frame, packet, outTimeRange);
				if (buffer)
				{
					_lastDecodedAudioChunkDisplayTime = outTimeRange.start;
				}
				return buffer;
			}
			else
			{
				continue;
			}
		}
	}
	return nullptr;
}

int FAudioDecoder::seek(FMediaTime time)
{
	FMediaTime seekTime = time;
	seekTime = seekTime.convertScale(audioStream->time_base.den);
	int seekResult = av_seek_frame(formatContext, audioStreamIndex, seekTime.timeValue(), AVSEEK_FLAG_BACKWARD);
	if (seekResult < 0)
	{
		return seekResult;
	}
	avcodec_flush_buffers(audioCodecCtx);
	return 1;
}

FMediaTime FAudioDecoder::lastDecodedAudioChunkDisplayTime() const
{
	return _lastDecodedAudioChunkDisplayTime;
}

FMediaTime FAudioDecoder::fps() const
{
	if (audioStream)
	{
		FMediaTime fps = FMediaTime(audioStream->avg_frame_rate);
		return fps;
	}
	return FMediaTime(-1.0, 600);
}

FAudioPCMBuffer * FAudioDecoder::newDecodedPCMBuffer(AVCodecContext* audioCodecCtx, AVFrame * frame, const AVPacket * packet, FMediaTimeRange & outTimeRange)
{
	int got_frame_ptr;
	int ret = avcodec_decode_audio4(audioCodecCtx, frame, &got_frame_ptr, packet);
	if (ret >= 0)
	{
		FAudioPCMBuffer* outPCMBuffer = new FAudioPCMBuffer(outputAudioFormat, frame->nb_samples);
		const uint8_t ** source = const_cast<const uint8_t **>(frame->data);
		ret = swr_convert(swrctx,
			outPCMBuffer->channelData(), frame->nb_samples,
			source, frame->nb_samples);
		outTimeRange = FMediaTimeRange(FMediaTime((int)frame->pts, frame->sample_rate), FMediaTime((int)frame->pts + (int)frame->nb_samples, frame->sample_rate));
		return outPCMBuffer;
	}
	else
	{
		return nullptr;
	}
}