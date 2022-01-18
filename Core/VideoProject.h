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

#ifndef VIDEOPROJECT_H
#define VIDEOPROJECT_H

#include <string>
#include <vector>
#include "ThirdParty/rapidjson.h"
#include "Vendor/noncopyable.hpp"
#include "Track/FTrack.h"
#include "VideoDescription.h"

class FVideoProject : public boost::noncopyable
{
private:
	std::string projectDir;
	std::string projectFilePath;

	FVideoDescription *videoDescription = nullptr;
	//std::vector<FImageTrack *> imageTracks;
	//std::vector<FAudioTrack *> audioTracks;

	void clean();
	FMediaTimeRange converTimeRange(rapidjson::GenericObject<false, rapidjson::Value> timeRange, int timeScale);
	bool loadVideoRenderContext(rapidjson::Document & doc, FVideoRenderContext& context);
	bool loadAudioRenderContext(rapidjson::Document & doc, FAudioRenderContext& context);
	bool loadVideoTracks(rapidjson::Document & doc);
	bool loadAudioTracks(rapidjson::Document & doc);

public:
	FVideoProject(const std::string& projectFilePath);
	~FVideoProject();

	bool prepare();

	const FVideoDescription *getVideoDescription() const;
	const std::vector<FImageTrack *> getImageTracks() const;
	const std::vector<FAudioTrack *> getAudioTracks() const;

	FImageTrack *insertNewImageTrack();
	FAudioTrack *insertNewAudioTrack();
};

#endif // VIDEOPROJECT_H