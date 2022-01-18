#pragma once
#include "ThirdParty/noncopyable.hpp"
#include "GLVertexBuffer.h"
#include "GLVertexBufferLayout.h"

class FGLVertexArray : public boost::noncopyable
{
private:
	unsigned int RendererID;
public:
	FGLVertexArray();
	~FGLVertexArray();

	int GetRendererID();

	void AddBuffer(const FGLVertexBuffer& Vb, const FGLVertexBufferLayout& Layout);

	void Bind() const;
	void Unbind();
};
