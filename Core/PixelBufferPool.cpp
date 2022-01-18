#include "PixelBufferPool.h"
#include <assert.h>

FPixelBufferPool::FPixelBufferPool(const unsigned int width, const unsigned int height, const unsigned int maxNum, const PixelBufferFormatType format)
	:width(width), height(height), maxNum(maxNum), format(format), cursor(0)
{
	assert(maxNum > 0);
	pixelBuffers.resize(maxNum);

	for (int i = 0; i < maxNum; i++)
	{
		pixelBuffers[i] = new FPixelBuffer(width, height, format);
	}
}

FPixelBufferPool::~FPixelBufferPool()
{
	for (int i = 0; i < maxNum; i++)
	{
		delete pixelBuffers[i];
	}
	pixelBuffers.clear();
}

FPixelBuffer * FPixelBufferPool::pixelBuffer()
{
	FPixelBuffer* pixelBuffer = pixelBuffers[cursor];
	cursor = cursor + 1;
	cursor = cursor % maxNum;
	return pixelBuffer;
}