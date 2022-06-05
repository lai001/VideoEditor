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

#ifndef VideoEditor_VideoTrack_hpp
#define VideoEditor_VideoTrack_hpp

#include <mutex>
#include <unordered_map>
#include <vector>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include "ImageTrack.hpp"

namespace ks
{
	struct FSourceFrame
	{
		PixelBuffer * sourceFrame = nullptr;
		MediaTime displayTime;
	};

	class FVideoTrack : public IImageTrack
	{
	public:
		FVideoTrack();
		~FVideoTrack();

	private:
		VideoDecoder *decoder = nullptr;

		std::vector<FSourceFrame> videoFrameQueue;

		std::mutex decoderMutex;

	public:
		std::string filePath;

	public:
		virtual const PixelBuffer * sourceFrame(const MediaTime & compositionTime, const FVideoRenderContext & renderContext) override;
		virtual const PixelBuffer * compositionImage(const PixelBuffer & sourceFrame, const MediaTime & compositionTime, const FVideoRenderContext & renderContext) override;
		virtual void prepare(const FVideoRenderContext & renderContext) override;
		virtual void onSeeking(const MediaTime & compositionTime) override;
		virtual void flush(const MediaTime & compositionTime) override;
		virtual void flush() override;
	};
}

#endif // VideoEditor_VideoTrack_hpp