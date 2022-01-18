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

#ifndef FIMAGETRACK_H
#define FIMAGETRACK_H

#include <string>
#include "Time/FTime.h"
#include "MediaTrack.h"
#include "RenderContext.h"
#include "PixelBuffer.h"

class FImageTrack: public IMediaTrack
{
public:
	virtual ~FImageTrack() 
	{
	};

public:
    virtual const FPixelBuffer *sourceFrame(const FMediaTime& compositionTime, const FVideoRenderContext& renderContext) = 0;
    virtual const FPixelBuffer *compositionImage(const FPixelBuffer& sourceFrame, const FMediaTime& compositionTime, const FVideoRenderContext& renderContext) = 0;
    virtual void prepare(const FVideoRenderContext& renderContext) = 0;
    virtual void onSeeking(const FMediaTime& compositionTime) = 0;
	virtual void flush(const FMediaTime& compositionTime) = 0;
	virtual void flush() = 0;
};

#endif // FIMAGETRACK_H
