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

#ifndef FUTIL_H
#define FUTIL_H

#include <functional>
#include <assert.h>
#include <algorithm>

#include "ThirdParty/FFmpeg.h"
#include "Time/FTime.h"
#include "AudioFormat.h"

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

namespace Bitmask
{
	template<class T>
	static bool isContains(T l, T r)
	{
		return (l & r) == r;
	}

	template<class T>
	static bool isNotContains(T l, T r)
	{
		return isContains(l, r) == false;
	}

	template<class T>
	static T insert(T l, T r)
	{
		return l | r;
	}

	template<class T>
	static T remove(T l, T r)
	{
		return l & ~r;
	}
}

class FUtil
{
public:
	FUtil();

	static std::function<double()> measue(bool fps = true);

	static std::string ffmpegErrorDescription(int errorCode);

	static std::string GetFolder(const std::string& Filename)
	{
		std::string Directory;
		const size_t LastSlashIndex = Filename.rfind('\\');
		if (std::string::npos != LastSlashIndex)
		{
			Directory = Filename.substr(0, LastSlashIndex);
		}
		return Directory;
	}

	static AVSampleFormat getAVSampleFormat(const FAudioFormat& format)
	{
		bool isSignedInteger = Bitmask::isContains(format.formatFlags, AudioFormatFlag::isSignedInteger);
		bool isFloat = Bitmask::isContains(format.formatFlags, AudioFormatFlag::isFloat);
		bool isNonInterleaved = Bitmask::isContains(format.formatFlags, AudioFormatFlag::isNonInterleaved);
		AVSampleFormat sampleFormat = AV_SAMPLE_FMT_NONE;

		if (isSignedInteger && format.bitsPerChannel == 16 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_S16P;
		}
		if (isSignedInteger && format.bitsPerChannel == 16 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_S16;
		}
		if (isSignedInteger && format.bitsPerChannel == 32 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_S32P;
		}
		if (isSignedInteger && format.bitsPerChannel == 32 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_S32;
		}
		if (isFloat && format.bitsPerChannel == 32 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_FLTP;
		}
		if (isFloat && format.bitsPerChannel == 32 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_FLT;
		}
		if (isFloat && format.bitsPerChannel == 64 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_DBLP;
		}
		if (isFloat && format.bitsPerChannel == 64 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_DBL;
		}
		assert(sampleFormat != AV_SAMPLE_FMT_NONE);
		return sampleFormat;
	}

	static FMediaTime getSourceTime(const FMediaTimeMapping& mapping, const FMediaTime& compositionTime)
	{
		auto a = compositionTime - mapping.target.start;
		auto p = a.seconds() / mapping.target.duration().seconds();
		auto start = p * mapping.source.duration().seconds() + mapping.source.start.seconds();
		return FMediaTime(start, 600);
	}

	static FMediaTime getTargetTime(const FMediaTimeMapping& mapping, const FMediaTime& sourceTime)
	{
		auto a = sourceTime - mapping.source.start;
		auto p = a.seconds() / mapping.source.duration().seconds();
		auto start = p * mapping.target.duration().seconds() + mapping.target.start.seconds();
		return FMediaTime(start, 600);
	}
};

#endif // FUTIL_H
