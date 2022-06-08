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

#ifndef VideoEditor_ImageTrack_hpp
#define VideoEditor_ImageTrack_hpp

#include <string>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include <KSImage/KSImage.hpp>
#include "MediaTrack.hpp"
#include "RenderContext.hpp"

namespace ks
{
	class IImageTrack : public IMediaTrack
	{
	public:
		ks::Rect rect;

	public:
		virtual ~IImageTrack() = 0 {};
		virtual const PixelBuffer *sourceFrame(const MediaTime& compositionTime, const VideoRenderContext& renderContext) = 0;
		virtual const PixelBuffer *compositionImage(const PixelBuffer& sourceFrame, const MediaTime& compositionTime, const VideoRenderContext& renderContext) = 0;
		virtual void prepare(const VideoRenderContext& renderContext) = 0;
		virtual void onSeeking(const MediaTime& compositionTime) = 0;
		virtual void flush(const MediaTime& compositionTime) = 0;
		virtual void flush() = 0;
	};
}

#endif // VideoEditor_ImageTrack_hpp