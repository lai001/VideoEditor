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

#ifndef VideoEditor_VideoProject_hpp
#define VideoEditor_VideoProject_hpp

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include <KSImage/KSImage.hpp>
#include "VideoTrack.hpp"
#include "AudioTrack.hpp"
#include "VideoDescription.hpp"

namespace ks
{
	class FVideoProject : public boost::noncopyable
	{
		typedef nlohmann::json Json;

	private:
		std::string projectDir;
		std::string projectFilePath;

		FVideoDescription *videoDescription = nullptr;
		//std::vector<IImageTrack *> imageTracks;
		//std::vector<FAudioTrack *> audioTracks;

		void clean();
		MediaTimeRange converTimeRange(const Json & json, int timeScale);
		Rect converRect(const Json & json);
		bool loadVideoRenderContext(const Json & json, FVideoRenderContext& context);
		bool loadAudioRenderContext(const Json & json, FAudioRenderContext& context);
		bool loadVideoTracks(const Json & json);
		bool loadAudioTracks(const Json & json);

	public:
		FVideoProject(const std::string& projectFilePath);
		~FVideoProject();

		bool prepare();

		const FVideoDescription *getVideoDescription() const;
		const std::vector<IImageTrack *> getImageTracks() const;
		const std::vector<FAudioTrack *> getAudioTracks() const;

		IImageTrack *insertNewImageTrack();
		FAudioTrack *insertNewAudioTrack();
	};
}

#endif // VideoEditor_VideoProject_hpp