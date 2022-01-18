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

#ifndef AUDIOPCMBUFFER_H
#define AUDIOPCMBUFFER_H

#include <string>
#include "Vendor/noncopyable.hpp"
#include "AudioFormat.h"
#include "Time/FTime.h"

class FAudioPCMBuffer : public boost::noncopyable
{
private:
	FAudioFormat _audioFormat;
	unsigned int _frameCapacity;
	unsigned char** _channelData = nullptr;

protected:
	FAudioPCMBuffer();

public:
	FAudioPCMBuffer(const FAudioFormat format, const unsigned int frameCapacity);
	~FAudioPCMBuffer();

	FAudioFormat audioFormat() const;
	unsigned int frameCapacity() const;
	unsigned int bytesPerSample() const;
	unsigned int bytesDataSizePerChannel() const;

	float **floatChannelData();
	short **int16ChannelData();
	unsigned short **uint16ChannelData();
	unsigned char ** channelData();

	const float * const *immutableFloatChannelData() const;

	void setZero();

	std::string debugDescription() const;
};

#endif // AUDIOPCMBUFFER_H