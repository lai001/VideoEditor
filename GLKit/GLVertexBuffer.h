#pragma once
#include "ThirdParty/noncopyable.hpp"
class FGLVertexBuffer: public boost::noncopyable
{
private:
	unsigned int RendererID;
public:
	FGLVertexBuffer(const void* Data, unsigned int Size);
	~FGLVertexBuffer();

	void Bind() const;
	void Unbind() const;
};

