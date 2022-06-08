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

#ifndef VideoEditor_VideoDescription_hpp
#define VideoEditor_VideoDescription_hpp

#include <vector>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include "MediaTrack.hpp"
#include "Resolution.hpp"
#include "VideoInstruction.hpp"
#include "RenderContext.hpp"
#include "ImageTrack.hpp"
#include "AudioTrack.hpp"

namespace ks
{
	class VideoDescription
	{
	public:
		VideoDescription();
		~VideoDescription();

		RenderContext renderContext;

		void prepare();

		static std::vector<MediaTimeRange> instructionTimeRanges(std::vector<MediaTimeRange> timeRanges);

		bool videoInstuction(const MediaTime time, VideoInstruction& outVideoInstruction) const;

		std::vector<IImageTrack *> imageTracks;
		std::vector<FAudioTrack *> audioTracks;

		MediaTime duration() const;

	protected:
		std::vector<VideoInstruction> videoInstructions;

	private:
		void removeAllVideoInstuctions();

		MediaTime _duration;
	};
}

#endif // VideoEditor_VideoDescription_hpp