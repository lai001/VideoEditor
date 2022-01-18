#include "GLIndexBuffer.h"
#include "ThirdParty/opengl.h"

FGLIndexBuffer::FGLIndexBuffer(const unsigned int* Data, unsigned int Count)
	: Count(Count)
{
	glGenBuffers(1, &RendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Count * sizeof(unsigned int), Data, GL_STATIC_DRAW);
}

FGLIndexBuffer::~FGLIndexBuffer()
{
	glDeleteBuffers(1, &RendererID);
}

void FGLIndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RendererID);
}

void FGLIndexBuffer::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
