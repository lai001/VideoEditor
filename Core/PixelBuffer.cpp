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

#include "PixelBuffer.h"

FPixelBuffer::FPixelBuffer(const unsigned int width, const unsigned int height, const PixelBufferFormatType formatType)
	:_width(width), _height(height), _formatType(formatType)
{

	unsigned char* imageData = new unsigned char[width * height * 4];

	_data.push_back(imageData);
}

FPixelBuffer::~FPixelBuffer()
{
	for (unsigned char* data : _data)
	{
		delete[] data;
	}
	_data.clear();
}

unsigned int FPixelBuffer::width() const
{
	return _width;
}

unsigned int FPixelBuffer::height() const
{
	return _height;
}

PixelBufferFormatType FPixelBuffer::formatType() const
{
	return _formatType;
}

std::vector<unsigned char*> FPixelBuffer::data()
{
	return _data;
}

std::vector<const unsigned char*> FPixelBuffer::immutableData() const
{
	std::vector<const unsigned char *> data = std::vector<const unsigned char*>();

	for (auto item : _data)
	{
		data.push_back(item);
	}

	return data;
}
