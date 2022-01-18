#pragma once
#include <functional>
#include <vector>
#include <array>
#include "ThirdParty/opengl.h"
#include "ThirdParty/noncopyable.hpp"
#include "GLVertexArray.h"
#include "GLShader.h"
#include "GLIndexBuffer.h"

class FGLVertexObject: public boost::noncopyable
{
public:

	FGLVertexObject(const void * VertexBufferData,
		const unsigned int VertexBufferDataSize,
		const unsigned int * IndexBufferData,
		const unsigned int IndicesCount,
		const std::function<const FGLVertexBufferLayout()> VertexBufferLayoutMaker);

	FGLVertexObject(const void * VertexBufferData,
		const unsigned int VertexBufferDataSize,
		const std::function<const FGLVertexBufferLayout()> VertexBufferLayoutMaker);


	~FGLVertexObject();

	void Bind() const;
	void Unbind() const;

private:
	FGLVertexArray* Va = nullptr;
	FGLVertexBuffer* Vb = nullptr;
	FGLIndexBuffer* Ib = nullptr;

};
