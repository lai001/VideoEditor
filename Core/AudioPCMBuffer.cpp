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

#include "AudioPCMBuffer.h"
#include <assert.h>
#include "Utility/FUtility.h"

FAudioPCMBuffer::FAudioPCMBuffer()
{
}

FAudioPCMBuffer::FAudioPCMBuffer(const FAudioFormat format, const unsigned int frameCapacity)
	: _audioFormat(format), _frameCapacity(frameCapacity)
{

	if (format.isNonInterleaved())
	{
		_channelData = new uint8_t *[format.channelsPerFrame];
		for (int i = 0; i < format.channelsPerFrame; i++)
		{
			int size = bytesDataSizePerChannel();
			_channelData[i] = new uint8_t[size];
			memset(_channelData[i], 0, size);
		}
	}
	else
	{
		_channelData = new uint8_t *[1];
		int size = bytesDataSizePerChannel();
		_channelData[0] = new uint8_t[size];
		memset(_channelData[0], 0, size);
	}
}

FAudioPCMBuffer::~FAudioPCMBuffer()
{
	if (_audioFormat.isNonInterleaved())
	{
		for (int i = 0; i < _audioFormat.channelsPerFrame; i++)
		{
			if (_channelData[i])
			{
				delete[] _channelData[i];
			}
		}
		delete[] _channelData;
	}
	else
	{
		delete[] _channelData[0];
		delete[] _channelData;
	}
}

FAudioFormat FAudioPCMBuffer::audioFormat() const
{
	return _audioFormat;
}

unsigned int FAudioPCMBuffer::frameCapacity() const
{
	return _frameCapacity;
}

unsigned int FAudioPCMBuffer::bytesPerSample() const
{
	return _audioFormat.bitsPerChannel / 8;
}

float **FAudioPCMBuffer::floatChannelData()
{
	return (float **)_channelData;
}

short **FAudioPCMBuffer::int16ChannelData()
{
	return (short **)_channelData;
}

unsigned short **FAudioPCMBuffer::uint16ChannelData()
{
	return (unsigned short **)_channelData;
}

unsigned char ** FAudioPCMBuffer::channelData()
{
	return _channelData;
}

const float * const * FAudioPCMBuffer::immutableFloatChannelData() const
{
	return (const float * const *)_channelData;
}

void FAudioPCMBuffer::setZero()
{
	bool isNonInterleaved = Bitmask::isContains(audioFormat().formatFlags, AudioFormatFlag::isNonInterleaved);

	if (isNonInterleaved)
	{
		for (int i = 0; i < audioFormat().channelsPerFrame; i++)
		{
			memset(channelData()[i], 0, bytesDataSizePerChannel());
		}
	}
	else
	{
		memset(channelData()[0], 0, bytesDataSizePerChannel());
	}
}

unsigned int FAudioPCMBuffer::bytesDataSizePerChannel() const
{
	return _audioFormat.bytesPerFrame * _frameCapacity;
}

std::string FAudioPCMBuffer::debugDescription() const
{
	return "";
}
