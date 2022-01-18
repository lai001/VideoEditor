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

#ifndef FVIDEODECODER_H
#define FVIDEODECODER_H

#include <string>
#include <vector>

#include "ThirdParty/FFmpeg.h"
#include "Vendor/noncopyable.hpp"
#include "PixelBuffer.h"
#include "Time/FTime.h"

class FVideoDecoder : public boost::noncopyable
{
protected:
	FVideoDecoder();

public:
	static FVideoDecoder* New(const std::string & filePath, const PixelBufferFormatType& formatType);
	~FVideoDecoder();

private:
	std::string filePath = "";

	AVFormatContext *formatContext = nullptr;
	AVStream *videoStream = nullptr;
	AVCodecContext *videoCodecCtx = nullptr;
	AVCodec *codec = nullptr;
	int videoStreamIndex = -1;
	struct SwsContext *imageSwsContext = nullptr;
	FMediaTime _lastDecodedImageDisplayTime = FMediaTime::zero;

private:
	FPixelBuffer* newDecodedFrame(AVCodecContext * videoCodecCtx, AVFrame* frame, AVPacket* packet, FMediaTime& outTime);

public:
	FPixelBuffer* newFrame(FMediaTime& outPts);
	bool seek(const FMediaTime& time);

	FMediaTime lastDecodedImageDisplayTime();
	FMediaTime fps();
};

#endif // FVIDEODECODER_H
