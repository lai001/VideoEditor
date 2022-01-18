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

#ifndef FVIDEODESCRIPTION_H
#define FVIDEODESCRIPTION_H

#include <vector>

#include "Track/FTrack.h"
#include "AudioFormat.h"
#include "Resolution.h"
#include "VideoInstruction.h"
#include "RenderContext.h"

class FVideoDescription
{
public:
	FVideoDescription();
	~FVideoDescription();

	FRenderContext renderContext;

	void prepare();

	static std::vector<FMediaTimeRange> instructionTimeRanges(std::vector<FMediaTimeRange> timeRanges);

	bool videoInstuction(const FMediaTime time, FVideoInstruction& outVideoInstruction) const;

	std::vector<FImageTrack *> imageTracks;
	std::vector<FAudioTrack *> audioTracks;

	FMediaTime duration() const;

protected:
	std::vector<FVideoInstruction> videoInstructions;

private:
	void removeAllVideoInstuctions();

	FMediaTime _duration;
};

#endif // FVIDEODESCRIPTION_H
