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

#ifndef FAUDIOTRACK_H
#define FAUDIOTRACK_H

#include <mutex> 
#include <vector>
#include <string>

#include "Time/FTime.h"
#include "AudioFormat.h"
#include "AudioDecoder.h"
#include "AudioPCMBuffer.h"
#include "RenderContext.h"
#include "MediaTrack.h"

struct AudioPCMBufferQueueItem
{
	FMediaTimeRange timeRange;
	FAudioPCMBuffer* pcmBuffer = nullptr;
};

class FAudioTrack : public IMediaTrack
{
public:
	FAudioTrack();
	~FAudioTrack();

public:
	std::string filePath;

public:
	virtual void prepare(const FAudioRenderContext& renderContext);
	virtual void flush();
	virtual void flush(const FMediaTime& time);
	virtual void onSeeking(const FMediaTime& time);
	virtual void samples(const FMediaTimeRange& timeRange, FAudioPCMBuffer* outAudioPCMBuffer);

private:
	std::vector<AudioPCMBufferQueueItem> bufferQueue;
	FAudioDecoder* decoder = nullptr;
	FAudioFormat outputAudioFormat;
	FMediaTime time;
	std::mutex decoderMutex;
};

#endif // FAUDIOTRACK_H
