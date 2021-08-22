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

#include <QtCore>
#include <QApplication>
#include <QFile>
#include <QSize>
#include <QImage>
#include <QPainter>
#include <functional>
#include <QFile>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FTime.h"
#include "ScopeGuard.h"
#include "Vendor/FFmpeg.h"
#include "Util.h"

FExportSession::FExportSession(FVideoDescription *des)
    :videoDescription(des)
{
    assert(des);
}

FExportSession::~FExportSession()
{

}

void FExportSession::start(std::function<void (int)> completionCallback)
{

    const QSize renderSize = videoDescription->renderSize;
    const float renderScale = videoDescription->renderScale;

    const QString filename = QApplication::applicationDirPath().append("/output.mp4");

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

    avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, filename.toStdString().c_str());
    outputFormat = outputFormatContext->oformat;

    videoCodec = avcodec_find_encoder(outputFormat->video_codec);
    videoStream = avformat_new_stream(outputFormatContext, videoCodec);
    videoCodecContext = avcodec_alloc_context3(videoCodec);

    audioCodec = avcodec_find_encoder(outputFormat->audio_codec);
    audioStream = avformat_new_stream(outputFormatContext, audioCodec);
    audioCodecContext = avcodec_alloc_context3(audioCodec);

    videoCodecContext->bit_rate = 40000000;
    videoCodecContext->width = renderSize.width() * renderScale;
    videoCodecContext->height = renderSize.height() * renderScale;
    videoCodecContext->framerate = FMediaTime(videoDescription->fps, 600).getRational();
    videoCodecContext->time_base = FMediaTime(videoDescription->fps, 600).invert().getRational();
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

    audioCodecContext->sample_fmt = audioCodec->sample_fmts ? audioCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    audioCodecContext->bit_rate = 64000;
    audioCodecContext->sample_rate = 44100;
    audioCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
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
                                         audioCodecContext->channel_layout, audioCodecContext->sample_fmt, audioCodecContext->sample_rate,
                                         AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                                         0, nullptr);
    swr_init(audioSwrContext);

    videoSwsContext = sws_getContext(videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_RGBA,
                                     videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_YUV420P,
                                     SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

//    av_dump_format(outputFormatContext, 0, filename.toStdString().c_str(), 1);


    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        avio_open(&outputFormatContext->pb, filename.toStdString().c_str(), AVIO_FLAG_WRITE);
    }
    avformat_write_header(outputFormatContext, nullptr);

    std::function<int(AVFrame *, AVCodecContext *, AVStream *)> encodeFrame = [outputFormatContext](AVFrame *frame, AVCodecContext *codecContext, AVStream *steam)
    {
        int ret = 0;
        ret = avcodec_send_frame(codecContext, frame);
        if (ret < 0)
        {
            qDebug() << "Error sending a frame for encoding: " << FUtil::ffmpegErrorDescription(ret);
            assert(false);
        }

        while (ret >= 0)
        {
            AVPacket pkt = {0};
            ret = avcodec_receive_packet(codecContext, &pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                qDebug() << "Error during encoding: " << FUtil::ffmpegErrorDescription(ret);
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

        while (1)
        {
            if (decodeImageTime.seconds() >= videoDescription->duration().seconds())
            {
                break;
            }

            const FVideoInstruction *videoInstuction = videoDescription->videoInstuction(decodeImageTime);
            defer
            {
                decodeImageTime = decodeImageTime + FMediaTime(1.0 / videoDescription->fps, videoCodecContext->time_base.den);
                decodeImageTime = decodeImageTime.convertScale(videoCodecContext->time_base.den);
                qDebug() << "Video: " << decodeImageTime.seconds();
            };

            QImage *outputImage = new QImage(renderSize, QImage::Format_RGBA8888);
            QPainter *painter = new QPainter(outputImage);
            defer
            {
                delete painter;
                painter = nullptr;

                delete outputImage;
                outputImage = nullptr;
            };

            int index = 0;
            for (FImageTrack *imageTrack : videoInstuction->imageTracks)
            {
                const FImage *image = imageTrack->sourceFrame(decodeImageTime, renderSize, renderScale);
                if (index == 0)
                {
                    painter->drawImage(QRect(0, 0, renderSize.width() / 2, renderSize.height() / 2),
                                       *image->image(),
                                       QRect(0, 0, image->image()->size().width(), image->image()->size().height()));
                }
                else
                {
                    painter->drawImage(QRect(renderSize.width() / 2, renderSize.height() / 2, renderSize.width() / 2, renderSize.height() / 2),
                                       *image->image(),
                                       QRect(0, 0, image->image()->size().width(), image->image()->size().height()));
                }
                index++;
            }

            int rgblinesizes[4];
            av_image_fill_linesizes(rgblinesizes, AV_PIX_FMT_RGBA, outputImage->width());

            uint8_t *src[1] = {outputImage->bits()};

            sws_scale(videoSwsContext, src,
                      rgblinesizes, 0, outputImage->height(),
                      frame->data, frame->linesize);

            frame->pts = decodeImageTime.timeValue();

            encodeFrame(frame, videoCodecContext, videoStream);

            for (auto imageTrack : videoDescription->imageTracks)
            {
                FMediaTimeRange cleanTimeRange = FMediaTimeRange(FMediaTime::zero, decodeImageTime);
                imageTrack->requestCleanCache(cleanTimeRange);
            }
        }

        encodeFrame(nullptr, videoCodecContext, videoStream);

        av_frame_unref(frame);
        av_frame_free(&frame);
    }

    {
        AVFrame *frame = av_frame_alloc();
        frame->format = audioCodecContext->sample_fmt;
        frame->channel_layout = audioCodecContext->channel_layout;
//        frame->channels = audioCodecContext->channels;
        frame->sample_rate = audioCodecContext->sample_rate;
        frame->nb_samples = audioCodecContext->frame_size == 0 ? 1024 : audioCodecContext->frame_size;
        av_frame_get_buffer(frame, 0);
        int ret;
        int pts = 0;
        int nextPts = 0;
        int dst_nb_samples = 0;
        FMediaTime decodeAudioTime = FMediaTime(0, 44100);
        const int sampleSize = videoDescription->audioFormat.sampleSize() / sizeof(uint8_t) / 8;
        int outputBufferSize = frame->nb_samples * sampleSize * audioCodecContext->channels;
        uint8_t* outputBuffer = new uint8_t[outputBufferSize];
        int samples_count = 0;

        defer {
            delete[] outputBuffer;
            outputBuffer = nullptr;
        };

        while (1)
        {
            if (decodeAudioTime.seconds() >= videoDescription->duration().seconds())
            {
                break;
            }

            memset(outputBuffer, 0, outputBufferSize);

            dst_nb_samples = av_rescale_rnd(swr_get_delay(audioSwrContext, audioCodecContext->sample_rate) + frame->nb_samples,
                                            audioCodecContext->sample_rate, audioCodecContext->sample_rate, AV_ROUND_UP);

            FMediaTime duration = FMediaTime(frame->nb_samples, videoDescription->audioFormat.sampleRate());
            const FVideoInstruction *videoInstuction = videoDescription->videoInstuction(decodeAudioTime);

            uint8_t* buffer = new uint8_t[outputBufferSize];
            defer {
                delete[] buffer;
                buffer = nullptr;
            };
            FMediaTimeRange timeRange = FMediaTimeRange(decodeAudioTime, decodeAudioTime + duration);
//                qDebug() << timeRange.debugDescription();
            for (FAudioTrack* audioTrack : videoInstuction->audioTracks)
            {
                memset(buffer, 0, outputBufferSize);
                audioTrack->samples(timeRange, outputBufferSize, buffer, videoDescription->audioFormat);
                for (int i = 0; i < outputBufferSize / sampleSize; i++)
                {
                    ((short*)outputBuffer)[i] = (((short*)outputBuffer)[i] + ((short*)buffer)[i]) / videoInstuction->audioTracks.size();
                }
            }

            const uint8_t* buffer1dsa[1] = {outputBuffer};

            swr_convert(audioSwrContext,
                        frame->data, dst_nb_samples,
                        buffer1dsa, frame->nb_samples);
            frame->pts = av_rescale_q(samples_count, (AVRational){1, audioCodecContext->sample_rate}, audioCodecContext->time_base);
            frame->pts = nextPts;
            nextPts = frame->pts + frame->nb_samples;
            samples_count += dst_nb_samples;
//            qDebug() << dst_nb_samples << duration.timeValue() << duration.timeScale() << frame->nb_samples;

            encodeFrame(frame, audioCodecContext, audioStream);

            decodeAudioTime = decodeAudioTime + duration;
            qDebug() << "audio " << decodeAudioTime.seconds();
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

    completionCallback(0);
}
