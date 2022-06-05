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

#include "ImageCompositionPipeline.hpp"
#include <assert.h>
#include <spdlog/spdlog.h>

namespace ks
{
	FImageCompositionPipeline::FImageCompositionPipeline()
	{
	}

	FImageCompositionPipeline::~FImageCompositionPipeline()
	{

	}

	void FImageCompositionPipeline::composition(FAsyncImageCompositionRequest& request, std::function<PixelBuffer*()> getPixelBuffer)
	{
		request.getPixelBuffer = [request, getPixelBuffer]()
		{
			PixelBuffer* pixelBuffer = getPixelBuffer();

			static std::unique_ptr<ks::FilterContext> context = std::unique_ptr<ks::FilterContext>(ks::FilterContext::create());

			const Rect renderRect = ks::Rect(0.0, 0.0, pixelBuffer->getWidth(), pixelBuffer->getHeight());
			std::vector<ks::Image*> images;
			for (auto args : request.sourceFrames)
			{
				ks::Image* image = ks::Image::createBorrow(args.second);
				images.push_back(image);
			}
			// TODO:
			assert(images.size() == 2);

			ks::Image* inputImage = images[0];
			ks::Image* inputTargetImage = images[1];

			std::shared_ptr<ks::SourceOverFilter> mixTwoImageFilter0 = std::shared_ptr<ks::SourceOverFilter>(ks::SourceOverFilter::create());

			std::shared_ptr<ks::TransformFilter> transformFilter0 = std::shared_ptr<ks::TransformFilter>(ks::TransformFilter::create());
			transformFilter0->inputImage = inputImage;
			transformFilter0->transform = ks::RectTransDescription(inputImage->getRect())
				.newRect(ks::Rect(0, 0, 640, 360))
				.getTransform();

			std::shared_ptr<ks::TransformFilter> transformFilter1 = std::shared_ptr<ks::TransformFilter>(ks::TransformFilter::create());
			transformFilter1->inputImage = inputTargetImage;
			transformFilter1->transform = ks::RectTransDescription(inputTargetImage->getRect())
				.newRect(ks::Rect(640, 360, 640, 360))
				.getTransform();

			mixTwoImageFilter0->inputImage = transformFilter0->outputImage();
			mixTwoImageFilter0->inputTargetImage = transformFilter1->outputImage();

			auto bufferPtr = std::shared_ptr<ks::PixelBuffer>(context->render(*mixTwoImageFilter0->outputImage(), renderRect));

			assert(bufferPtr->getWidth() == pixelBuffer->getWidth());
			assert(bufferPtr->getHeight() == pixelBuffer->getHeight());
			memcpy(pixelBuffer->getMutableData()[0],
				bufferPtr->getImmutableData()[0],
				4 * pixelBuffer->getWidth() * pixelBuffer->getHeight());

			return pixelBuffer;
		};
	}
}