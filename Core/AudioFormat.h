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

#ifndef AUDIOFORMAT_H
#define AUDIOFORMAT_H
#include <assert.h>
#include <unordered_map>

enum class AudioFormatIdentifiersType
{
	pcm
};

enum class AudioSampleType
{
	uint16,
	uint32,
	sint16,
	sint32,
	float32,
	float64
};

static std::string AudioSampleTypeToString(const AudioSampleType type)
{
	std::unordered_map<AudioSampleType, std::string> table;
	table[AudioSampleType::uint16] = "uint16";
	table[AudioSampleType::uint32] = "uint32";
	table[AudioSampleType::sint16] = "sint16";
	table[AudioSampleType::sint32] = "sint32";
	table[AudioSampleType::float32] = "float32";
	table[AudioSampleType::float64] = "float64";
	return table[type];
}

namespace AudioFormatFlag
{
	static unsigned int isFloat = 1 << 1;
	static unsigned int isSignedInteger = 1 << 2;
	static unsigned int isNonInterleaved = 1 << 3;
	static unsigned int isBigEndian = 1 << 4;
}

struct FAudioFormat
{
	double                     sampleRate;
	AudioFormatIdentifiersType formatType;
	unsigned int               formatFlags;
	unsigned int               bytesPerPacket;
	unsigned int               framesPerPacket;
	unsigned int               bytesPerFrame;
	unsigned int               channelsPerFrame;
	unsigned int               bitsPerChannel;

	bool operator==(const FAudioFormat &format) const;

	AudioSampleType getSampleType() const;

	std::string debugDescription() const;

	bool isNonInterleaved() const;
	bool isFloat() const;
	bool isSignedInteger() const;

};

#endif // AUDIOFORMAT_H