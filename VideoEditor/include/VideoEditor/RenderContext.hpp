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

#ifndef VideoEditor_RenderContext_hpp
#define VideoEditor_RenderContext_hpp

#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include "Resolution.hpp"

namespace ks
{
	class FVideoRenderContext
	{
	public:
		FSize renderSize;
		float renderScale;
		float fps;
		PixelBuffer::FormatType format;

		FVideoRenderContext() = default;
		~FVideoRenderContext() = default;
	};

	class FAudioRenderContext
	{
	public:
		AudioFormat audioFormat;

		FAudioRenderContext() = default;
		~FAudioRenderContext() = default;
	};

	class FRenderContext
	{
	public:
		FVideoRenderContext videoRenderContext;
		FAudioRenderContext audioRenderContext;
		FRenderContext() = default;
		~FRenderContext() = default;

	};
}

#endif // VideoEditor_RenderContext_hpp