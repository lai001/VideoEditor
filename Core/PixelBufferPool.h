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

#ifndef FPIXELBUFFERPOOL_H
#define FPIXELBUFFERPOOL_H

#include <vector>

#include "Vendor/noncopyable.hpp"
#include "PixelBuffer.h"

class FPixelBufferPool : public boost::noncopyable
{
private:
	std::vector<FPixelBuffer*> pixelBuffers;
	unsigned int width;
	unsigned int height;
	unsigned int maxNum;
	PixelBufferFormatType format;

	unsigned int cursor;

public:
	FPixelBufferPool(const unsigned int width, const unsigned int height, const unsigned int maxNum, const PixelBufferFormatType format);
	~FPixelBufferPool();

	FPixelBuffer* pixelBuffer();
};

#endif // FPIXELBUFFERPOOL_H
