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

#ifndef VideoEditor_ExportSession_hpp
#define VideoEditor_ExportSession_hpp

#include <string>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include "VideoDescription.hpp"
#include "ImageCompositionPipeline.hpp"

namespace ks
{
	class ExportSession
	{
	public:
		enum EncodeType
		{
			video,
			audio
		};

	public:
		ExportSession(const VideoDescription& videoDescription, ImageCompositionPipeline& imageCompositionPipeline);
		~ExportSession();
		void start(const std::string& filename,
			std::function<void(const ExportSession::EncodeType& type, const MediaTime& time)> progressCallback);

	private:
		const VideoDescription *videoDescription = nullptr;
		ImageCompositionPipeline *imageCompositionPipeline = nullptr;
	};
}

#endif // VideoEditor_ExportSession_hpp