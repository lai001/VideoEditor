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

#ifndef VIDEOTRACK_H
#define VIDEOTRACK_H

#include <mutex>
#include <unordered_map>
#include <vector>

#include "ImageTrack.h"
#include "VideoDecoder.h"

struct FSourceFrame
{
	FPixelBuffer * sourceFrame = nullptr;
	FMediaTime displayTime;
};

class FVideoTrack : public FImageTrack
{
public:
	FVideoTrack();
	~FVideoTrack();

private:
	FVideoDecoder *decoder = nullptr;

	std::vector<FSourceFrame> videoFrameQueue;

	std::mutex decoderMutex;

public:
	std::string filePath;

public:
	virtual const FPixelBuffer * sourceFrame(const FMediaTime & compositionTime, const FVideoRenderContext & renderContext) override;
	virtual const FPixelBuffer * compositionImage(const FPixelBuffer & sourceFrame, const FMediaTime & compositionTime, const FVideoRenderContext & renderContext) override;
	virtual void prepare(const FVideoRenderContext & renderContext) override;
	virtual void onSeeking(const FMediaTime & compositionTime) override;
	virtual void flush(const FMediaTime & compositionTime) override;
	virtual void flush() override;
};

#endif // VIDEOTRACK_H
