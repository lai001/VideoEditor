#include "GLVertexBuffer.h"
#include <assert.h>
#include "ThirdParty/opengl.h"

FGLVertexBuffer::FGLVertexBuffer(const void * Data, unsigned int Size)
{
	assert(Data);
	glGenBuffers(1, &RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, RendererID);
	glBufferData(GL_ARRAY_BUFFER, Size, Data, GL_STATIC_DRAW);
}

FGLVertexBuffer::~FGLVertexBuffer()
{
	glDeleteBuffers(1, &RendererID);
}

void FGLVertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, RendererID);
}

void FGLVertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
