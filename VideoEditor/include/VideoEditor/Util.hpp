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

#ifndef VideoEditor_Util_hpp
#define VideoEditor_Util_hpp

#include <functional>
#include <assert.h>
#include <algorithm>
#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include <KSImage/KSImage.hpp>

namespace ks
{
	struct FSize
	{
		float width;
		float height;

		FSize()
			:width(0), height(0)
		{

		}

		template<class T>
		FSize(T _width, T _height)
			: width(static_cast<float>(_width)), height(static_cast<float>(_height))
		{

		}
	};
}

namespace ks
{
	static std::string getFolder(const std::string& fileNmae)
	{
		std::string Directory;
		const size_t LastSlashIndex = fileNmae.rfind('\\');
		if (std::string::npos != LastSlashIndex)
		{
			Directory = fileNmae.substr(0, LastSlashIndex);
		}
		return Directory;
	}

	static MediaTime getSourceTime(const MediaTimeMapping& mapping, const MediaTime& compositionTime)
	{
		auto a = compositionTime - mapping.target.start;
		auto p = a.seconds() / mapping.target.duration().seconds();
		auto start = p * mapping.source.duration().seconds() + mapping.source.start.seconds();
		return MediaTime(start, 600);
	}

	static MediaTime getTargetTime(const MediaTimeMapping& mapping, const MediaTime& sourceTime)
	{
		auto a = sourceTime - mapping.source.start;
		auto p = a.seconds() / mapping.source.duration().seconds();
		auto start = p * mapping.target.duration().seconds() + mapping.target.start.seconds();
		return MediaTime(start, 600);
	}

	void InitVideoEditor(ks::IRenderEngine * renderEngine) noexcept;
}

#endif // VideoEditor_Util_hpp