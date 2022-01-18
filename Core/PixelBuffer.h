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

#ifndef FPIXELBUFFER_H
#define FPIXELBUFFER_H

#include <vector>

#include "Vendor/noncopyable.hpp"


enum class PixelBufferFormatType
{
	rgba32,
	bgra32
};

class FPixelBuffer : public boost::noncopyable
{
public:
	FPixelBuffer(const unsigned int width, const unsigned int height, const PixelBufferFormatType formatType);
	~FPixelBuffer();

public:
	unsigned int width() const;
	unsigned int height() const;
	PixelBufferFormatType formatType() const;
	std::vector<unsigned char*> data();
	std::vector<const unsigned char*> immutableData() const;

private:
	unsigned int _width;
	unsigned int _height;
	PixelBufferFormatType _formatType;

	std::vector<unsigned char*> _data;

};

#endif // FPIXELBUFFER_H
