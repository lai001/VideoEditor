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


#include "ImageCompositionPipeline.h"

FImageCompositionPipeline::FImageCompositionPipeline()
{
}

FImageCompositionPipeline::~FImageCompositionPipeline()
{

}

void FImageCompositionPipeline::composition(FAsyncImageCompositionRequest& request, std::function<FPixelBuffer*()> getPixelBuffer)
{
	// TODO:
	FPixelBuffer* pixelBuffer = getPixelBuffer();
	for (auto args : request.sourceFrames)
	{
		assert(args.second->width() == pixelBuffer->width());
		assert(args.second->height() == pixelBuffer->height());
		memcpy(pixelBuffer->data()[0], args.second->immutableData()[0], 4 * pixelBuffer->width() * pixelBuffer->height());
	}
	request.pixelBuffer = pixelBuffer;
}