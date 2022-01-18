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

#include "VideoDecoder.h"
#include <unordered_map>
#include <assert.h>
#include "Utility/FUtility.h"

FVideoDecoder * FVideoDecoder::New(const std::string & filePath, const PixelBufferFormatType& formatType)
{
	AVFormatContext *formatContext = nullptr;
	AVStream *videoStream = nullptr;
	AVCodecContext *videoCodecCtx = nullptr;
	AVCodec *codec = nullptr;
	int videoStreamIndex = -1;
	struct SwsContext *imageSwsContext = nullptr;

	do
	{
		formatContext = avformat_alloc_context();

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
			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				videoStreamIndex = i;
				videoStream = formatContext->streams[i];
				break;
			}
		}
		if (videoStreamIndex == -1)
		{
			break;
		}

		codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
		if (codec == nullptr)
		{
			break;
		}
		videoCodecCtx = avcodec_alloc_context3(codec);
		if (videoCodecCtx == nullptr)
		{
			break;
		}
		avcodec_parameters_to_context(videoCodecCtx, videoStream->codecpar);

		if (avcodec_open2(videoCodecCtx, codec, NULL) < 0)
		{
			break;
		}

		std::unordered_map<PixelBufferFormatType, AVPixelFormat> dic;
		dic[PixelBufferFormatType::rgba32] = AV_PIX_FMT_RGBA;
		dic[PixelBufferFormatType::bgra32] = AV_PIX_FMT_BGRA;

		imageSwsContext = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
			videoCodecCtx->width, videoCodecCtx->height, dic[formatType], SWS_FAST_BILINEAR, NULL, NULL, NULL);
		if (imageSwsContext == nullptr)
		{
			break;
		}

		FVideoDecoder * decoder = new FVideoDecoder();
		decoder->filePath = filePath;
		decoder->videoCodecCtx = videoCodecCtx;
		decoder->formatContext = formatContext;
		decoder->imageSwsContext = imageSwsContext;
		decoder->codec = codec;
		decoder->videoStream = videoStream;
		decoder->videoStreamIndex = videoStreamIndex;
		return decoder;
	} while (true);


	if (imageSwsContext)
	{
		sws_freeContext(imageSwsContext);
	}
	if (videoCodecCtx)
	{
		avcodec_close(videoCodecCtx);
		avcodec_free_context(&videoCodecCtx);
	}
	if (formatContext)
	{
		avformat_close_input(&formatContext);
		avformat_free_context(formatContext);
	}

	return nullptr;
}

FVideoDecoder::FVideoDecoder()
{
}

FVideoDecoder::~FVideoDecoder()
{
	sws_freeContext(imageSwsContext);

	avcodec_close(videoCodecCtx);
	avcodec_free_context(&videoCodecCtx);

	avformat_close_input(&formatContext);
	avformat_free_context(formatContext);
}

FPixelBuffer * FVideoDecoder::newDecodedFrame(AVCodecContext * videoCodecCtx, AVFrame* frame, AVPacket* packet, FMediaTime& outTime)
{
	int gotPicture;
	int ret = avcodec_decode_video2(videoCodecCtx, frame, &gotPicture, packet);

	if (ret < 0)
	{
		return nullptr;
	}
	else
	{
		FPixelBuffer* outPixelBuffer = new FPixelBuffer(videoCodecCtx->width, videoCodecCtx->height, PixelBufferFormatType::rgba32);
		unsigned char *outImageData = outPixelBuffer->data()[0];
		int linesizes[4];
		av_image_fill_linesizes(linesizes, AV_PIX_FMT_BGRA, frame->width);
		sws_scale(imageSwsContext, (const unsigned char *const *)frame->data, frame->linesize, 0, videoCodecCtx->height,
			&outImageData, linesizes);
		outTime = FMediaTime((int)frame->pts, videoStream->time_base.den);
		return outPixelBuffer;
	}
}

FPixelBuffer* FVideoDecoder::newFrame(FMediaTime& outPts)
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

		if (av_read_frame(formatContext, packet) < 0)
		{
			break;
		}
		else
		{
			if (packet->stream_index == videoStreamIndex)
			{
				FPixelBuffer* pixelBuffer = newDecodedFrame(videoCodecCtx, frame, packet, outPts);
				if (pixelBuffer)
				{
					_lastDecodedImageDisplayTime = outPts;
				}
				return pixelBuffer;
			}
			else
			{
				continue;
			}
		}
	}

	return nullptr;
}

bool FVideoDecoder::seek(const FMediaTime& time)
{
	FMediaTime seekTime = time;
	seekTime = seekTime.convertScale(videoStream->time_base.den);
	int seekResult = av_seek_frame(formatContext, videoStreamIndex, seekTime.timeValue(), AVSEEK_FLAG_BACKWARD);
	if (seekResult < 0)
	{
		return false;
	}
	avcodec_flush_buffers(videoCodecCtx);
	return true;
}

FMediaTime FVideoDecoder::lastDecodedImageDisplayTime()
{
	return _lastDecodedImageDisplayTime;
}

FMediaTime FVideoDecoder::fps()
{
	if (videoStream)
	{
		FMediaTime fps = FMediaTime(videoStream->avg_frame_rate);
		return fps;
	}
	return FMediaTime(-1.0, 600);
}