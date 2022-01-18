#pragma once
#include <vector>
#include <assert.h>
#include "ThirdParty/opengl.h"

struct FGLVertexBufferElement
{
	unsigned int Type;
	unsigned int Count;
	bool bNormalized;

	static unsigned int GetSizeOfType(unsigned int Type)
	{
		switch (Type)
		{

		case GL_FLOAT:				return 4;
		case GL_UNSIGNED_INT:		return 4;
		case GL_UNSIGNED_BYTE:		return 1;
		case GL_INT:				return 4;
		default:
			assert(false);
		}
	}
};

class FGLVertexBufferLayout
{

private:
	std::vector<FGLVertexBufferElement> Elements;
	unsigned int Stride = 0;

public:

	template<typename T>
	void Push(unsigned int Count)
	{
		static_assert(false);
	}

	template<>
	void Push<int>(unsigned int Count)
	{
		Elements.push_back({ GL_INT, Count, false });
		Stride += Count * sizeof(int);
	}

	template<>
	void Push<float>(unsigned int Count)
	{
		Elements.push_back({ GL_FLOAT, Count, false });
		Stride += Count * sizeof(float);
	}

	template<>
	void Push<unsigned int>(unsigned int Count)
	{
		Elements.push_back({ GL_UNSIGNED_INT, Count, false });
		Stride += Count * sizeof(unsigned int);
	}

	template<>
	void Push<unsigned char>(unsigned int Count)
	{
		Elements.push_back({ GL_UNSIGNED_BYTE, Count, false });
		Stride += Count * sizeof(unsigned char);
	}

	FGLVertexBufferLayout& Int(unsigned int Count)
	{
		Elements.push_back({ GL_INT, Count, false });
		Stride += Count * sizeof(int);
		return *this;
	}

	FGLVertexBufferLayout& Float(unsigned int Count)
	{
		Elements.push_back({ GL_FLOAT, Count, false });
		Stride += Count * sizeof(float);
		return *this;
	}

	FGLVertexBufferLayout& UnsignedInt(unsigned int Count)
	{
		Elements.push_back({ GL_UNSIGNED_INT, Count, false });
		Stride += Count * sizeof(unsigned int);
		return *this;
	}

	FGLVertexBufferLayout& UnsignedByte(unsigned int Count)
	{
		Elements.push_back({ GL_UNSIGNED_BYTE, Count, false });
		Stride += Count * sizeof(unsigned char);
		return *this;
	}

	inline std::vector<FGLVertexBufferElement> GetElements() const
	{
		return Elements;
	}

	inline unsigned int GetStride() const
	{
		return Stride;
	}
};