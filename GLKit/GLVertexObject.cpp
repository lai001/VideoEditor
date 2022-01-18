#include "GLVertexObject.h"

FGLVertexObject::FGLVertexObject(const void * VertexBufferData,
	const unsigned int VertexBufferDataSize,
	const unsigned int* IndexBufferData,
	const unsigned int IndicesCount,
	const std::function<const FGLVertexBufferLayout()> VertexBufferLayoutMaker)
{
	assert(VertexBufferData);
	assert(VertexBufferDataSize > 0);
	assert(IndexBufferData);
	assert(IndicesCount > 0);
	Va = new FGLVertexArray();
	Vb = new FGLVertexBuffer(VertexBufferData, VertexBufferDataSize);
	Ib = new FGLIndexBuffer(IndexBufferData, IndicesCount);
	const FGLVertexBufferLayout& Layout = VertexBufferLayoutMaker();
	Va->AddBuffer(*Vb, Layout);
}

FGLVertexObject::FGLVertexObject(const void * VertexBufferData,
	const unsigned int VertexBufferDataSize,
	const std::function<const FGLVertexBufferLayout()> VertexBufferLayoutMaker)
{
	assert(VertexBufferData);
	assert(VertexBufferDataSize > 0);
	Va = new FGLVertexArray();
	Vb = new FGLVertexBuffer(VertexBufferData, VertexBufferDataSize);
	const FGLVertexBufferLayout& Layout = VertexBufferLayoutMaker();
	Va->AddBuffer(*Vb, Layout);
}

FGLVertexObject::~FGLVertexObject()
{
	if (Ib)
	{
		delete Ib;
	}
	delete Vb;
	delete Va;
}

void FGLVertexObject::Bind() const
{
	Va->Bind();
}

void FGLVertexObject::Unbind() const
{
	Va->Unbind();
}
