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
#include "FrameWrapper.h"

FVideoDecoder::FVideoDecoder(QString filePath)
    : filePath(filePath)
{
    formatContext = avformat_alloc_context();

    if (avformat_open_input(&formatContext, filePath.toStdString().c_str(), NULL, NULL) != 0)
    {
        qDebug("Couldn't open input stream.");
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0)
    {
        qDebug("Couldn't find stream information.");
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

    codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    videoCodecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(videoCodecCtx, videoStream->codecpar);

    if (codec == nullptr)
    {
        qDebug("Codec not found.");
    }

    if (avcodec_open2(videoCodecCtx, codec, NULL) < 0)
    {
        qDebug("Could not open codec.");
    }

    imageSwsContext = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
                                     videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
}

FVideoDecoder::~FVideoDecoder()
{
    if (imageSwsContext)
    {
        sws_freeContext(imageSwsContext);
        imageSwsContext = nullptr;
    }
    if (videoCodecCtx)
    {
        avcodec_close(videoCodecCtx);
        avcodec_free_context(&videoCodecCtx);
        videoCodecCtx = nullptr;
    }
    if (formatContext)
    {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
}

int FVideoDecoder::frameAtTime(FMediaTime time, FVideoFrame *videoFrame)
{
    QMutexLocker locker(&mutex);
    if (videoFrame == nullptr)
    {
        return -1;
    }
    FMediaTime seekTime = time;

    AVFrame *frame = av_frame_alloc();

    AVPacket *packet = av_packet_alloc();

    seekTime = seekTime.convertScale(videoStream->time_base.den);

    int gotPicture;
    int ret;

    while (1)
    {
        FFrameWrapper wrapper = FFrameWrapper(packet, frame);

        if (av_read_frame(formatContext, packet) >= 0)
        {
            if (packet->stream_index == videoStreamIndex)
            {
                ret = avcodec_decode_video2(videoCodecCtx, frame, &gotPicture, packet);

                FMediaTime displayTime((int)frame->pts, videoStream->time_base.den);

                if (ret < 0)
                {
                    qDebug("Decode Error.\n");

                    break;
                }
                if (gotPicture && displayTime.seconds() >= seekTime.seconds())
                {
                    uint8_t *dst[1] = {videoFrame->fillImage(videoCodecCtx->width, videoCodecCtx->height, QImage::Format_RGB32)};

                    int linesizes[4];
                    av_image_fill_linesizes(linesizes, AV_PIX_FMT_RGBA, frame->width);
                    sws_scale(imageSwsContext, (const unsigned char *const *)frame->data, frame->linesize, 0, videoCodecCtx->height,
                              dst, linesizes);
                    videoFrame->displayTime = displayTime;
                    _lastDecodedImageDisplayTime = displayTime;

                    break;
                }
            }
        }
        else
        {
            break;
        }
    }

    av_packet_free(&packet);
    av_frame_free(&frame);
    return 1;
}

int FVideoDecoder::seek(FMediaTime time)
{
    QMutexLocker locker(&mutex);
    FMediaTime seekTime = time;
    seekTime = seekTime.convertScale(videoStream->time_base.den);
    int seekResult = av_seek_frame(formatContext, videoStreamIndex, seekTime.timeValue(), AVSEEK_FLAG_BACKWARD);
    if (seekResult < 0)
    {
        qErrnoWarning("seek failed! %.2f", seekTime.seconds());

        return seekResult;
    }
    //    qDebug() << "start seek frame at time: " << time.debugDescription();
    avcodec_flush_buffers(videoCodecCtx);
    return 1;
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