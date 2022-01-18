#pragma once
#include "ThirdParty/noncopyable.hpp"

class FGLIndexBuffer: public boost::noncopyable
{
private:
	unsigned int RendererID;
	unsigned int Count;

public:
	FGLIndexBuffer(const unsigned int* Data, unsigned int Count);
	~FGLIndexBuffer();

	void Bind();
	void Unbind();

	inline unsigned int GetCount() const
	{
		return Count;
	}
};

