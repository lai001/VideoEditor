#include "GLVertexArray.h"

FGLVertexArray::FGLVertexArray()
	: RendererID(0)
{
	glGenVertexArrays(1, &RendererID);
	glBindVertexArray(RendererID);
}

FGLVertexArray::~FGLVertexArray()
{
	glDeleteVertexArrays(1, &RendererID);
}

int FGLVertexArray::GetRendererID()
{
	return RendererID;
}

void FGLVertexArray::AddBuffer(const FGLVertexBuffer& Vb, const FGLVertexBufferLayout& Layout)
{
	glBindVertexArray(RendererID);
	Vb.Bind();
	const std::vector<FGLVertexBufferElement> Elements = Layout.GetElements();
	unsigned int Offset = 0;
	for (unsigned int i = 0; i < Elements.size(); i++)
	{
		FGLVertexBufferElement Element = Elements[i];
		glEnableVertexAttribArray(i);
		if (Element.Type == GL_INT)
		{
			glVertexAttribIPointer(i, Element.Count, Element.Type, Layout.GetStride(), (const void*)Offset);
			Offset += Element.Count * FGLVertexBufferElement::GetSizeOfType(Element.Type);
		}
		else
		{
			glVertexAttribPointer(i, Element.Count, Element.Type, Element.bNormalized, Layout.GetStride(), (const void*)Offset);
			Offset += Element.Count * FGLVertexBufferElement::GetSizeOfType(Element.Type);
		}

	}
}

void FGLVertexArray::Bind() const
{
	glBindVertexArray(RendererID);
}

void FGLVertexArray::Unbind()
{
	glBindVertexArray(0);
}
