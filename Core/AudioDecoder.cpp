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
#include "FrameWrapper.h"

FAudioDecoder::FAudioDecoder(QString filePath, QAudioFormat format)
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
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            audioStream = formatContext->streams[i];
            break;
        }
    }

    codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    audioCodecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(audioCodecCtx, audioStream->codecpar);

    if (codec == nullptr)
    {
        qDebug("Codec not found.");
    }

    if (avcodec_open2(audioCodecCtx, codec, NULL) < 0)
    {
        qDebug("Could not open codec.");
    }

    AVSampleFormat sampleFormat = AV_SAMPLE_FMT_NONE;

    switch (format.sampleType()) {
    case QAudioFormat::SampleType::SignedInt:
        sampleFormat = AV_SAMPLE_FMT_S16;
        break;
    case QAudioFormat::SampleType::UnSignedInt:
        sampleFormat = AV_SAMPLE_FMT_S16;
        break;
    case QAudioFormat::SampleType::Float:
        sampleFormat = AV_SAMPLE_FMT_FLT;
        break;
    default:
        break;
    }
    outputSampleFormat = sampleFormat;
    swrctx = swr_alloc_set_opts(swrctx,
                                av_get_default_channel_layout(format.channelCount()), sampleFormat, format.sampleRate(),
                                audioCodecCtx->channel_layout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate,
                                0, nullptr);
    swr_init(swrctx);

    //    qint64 destMs = audioStream->duration / audioStream->time_base.den;
    //    qDebug()<<"bit_rate:"<<audioCodecCtx->bit_rate;
    //    qDebug()<<"sample_fmt:"<<av_get_sample_fmt_name(audioCodecCtx->sample_fmt);
    //    qDebug()<<"channels:"<<audioCodecCtx->channels;
    //    qDebug()<<"sample_rate:"<<audioCodecCtx->sample_rate;
    //    qDebug()<<"duration (seconds):"<<destMs;
    //    qDebug()<<"decode name:"<<codec->name;
}

FAudioDecoder::~FAudioDecoder()
{
    if (swrctx)
    {
        swr_close(swrctx);
        swr_free(&swrctx);
        swrctx = nullptr;
    }
    if (audioCodecCtx)
    {
        avcodec_close(audioCodecCtx);
        avcodec_free_context(&audioCodecCtx);
        audioCodecCtx = nullptr;
    }
    if (formatContext)
    {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
}

int FAudioDecoder::frame(FAudioBuffer* audioBuffer)
{
    AVFrame *frame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    int fRet = -1;
    while (true)
    {
        FFrameWrapper wrapper = FFrameWrapper(packet, frame);
        int ret = av_read_frame(formatContext, packet);
        if (ret >= 0)
        {
            if (packet->stream_index == audioStreamIndex)
            {
                int ret = avcodec_decode_audio4(audioCodecCtx, frame, &ret, packet);
                if (ret >= 0)
                {
                    double pts = (double)frame->pts / (double)audioStream->time_base.den;
                    double duration = (double)frame->nb_samples / (double)frame->sample_rate;

                    int bytesPerSample = av_get_bytes_per_sample(outputSampleFormat);
                    int byteCount = frame->nb_samples * bytesPerSample * av_get_channel_layout_nb_channels(frame->channel_layout);

                    uint8_t* pcm = new uint8_t[byteCount];
                    ret = swr_convert(swrctx,
                                      &pcm, frame->nb_samples,                          
                                      (const uint8_t **)frame->data, frame->nb_samples); 

                    audioBuffer->byteCount = byteCount;
                    audioBuffer->pcmBuffer = pcm;
                    audioBuffer->numberSamples = frame->nb_samples;
                    audioBuffer->numberchannels = av_get_channel_layout_nb_channels(frame->channel_layout);
                    audioBuffer->sampleRate = frame->sample_rate;
                    audioBuffer->timeRange = FMediaTimeRange(FMediaTime(pts, frame->sample_rate), FMediaTime(pts + duration, frame->sample_rate));
                    _lastDecodedAudioChunkDisplayTime = FMediaTime((int)frame->pts, (int)audioStream->time_base.den);
                    fRet = 1;
                    break;
                }
                else
                {
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
    return fRet;
}

int FAudioDecoder::seek(FMediaTime time)
{
    QMutexLocker locker(&mutex);
    FMediaTime seekTime = time;
    seekTime = seekTime.convertScale(audioStream->time_base.den);
    int seekResult = av_seek_frame(formatContext, audioStreamIndex, seekTime.timeValue(), AVSEEK_FLAG_BACKWARD);
    if (seekResult < 0)
    {
        qErrnoWarning("seek failed! %.2f", seekTime.seconds());

        return seekResult;
    }
    //    qDebug() << "start seek frame at time: " << time.debugDescription();
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
