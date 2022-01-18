#include "GLTexture.h"
#include <iostream>
#include <assert.h>

FGLTexture::FGLTexture(const GLTextureImageTargetType TextureImageTargetType,
						const int Width,
						const int Height,
						const GLInternalFormatType InternalFormatType, 
						const GLPixelFormatType PixelFormatType,
						const int Level,
						const GLDataType DataType,
						bool IsGenerateMipmap, 
						const std::vector<const void*> Buffers)
	:Width(Width), Height(Height), TextureImageTargetType(TextureImageTargetType), RendererID(0)
{
	assert(Level >= 0);
	if (GL_TEXTURE_RECTANGLE == static_cast<int>(TextureImageTargetType) || GL_PROXY_TEXTURE_RECTANGLE == static_cast<int>(TextureImageTargetType))
	{
		assert(Level == 0);
	}
	if (GL_TEXTURE_CUBE_MAP == static_cast<int>(TextureImageTargetType))
	{
		assert(Buffers.size() == 6);
	}
	else
	{
		assert(Buffers.size() == 1 || Buffers.size() == 0);
	}
	for (const void* Buffer : Buffers)
	{
		assert(Buffer);
	}
	glGenTextures(1, &RendererID);
	glBindTexture(static_cast<int>(TextureImageTargetType), RendererID);
	if (GL_TEXTURE_CUBE_MAP == static_cast<int>(TextureImageTargetType))
	{
		for (int i = 0; i < Buffers.size(); i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Level, static_cast<int>(InternalFormatType), Width, Height, 0, static_cast<int>(PixelFormatType), static_cast<int>(DataType), Buffers[i]);
		}
	}
	else
	{
		if (Buffers.size() == 0)
		{
			glTexImage2D(static_cast<int>(TextureImageTargetType), Level, static_cast<int>(InternalFormatType), Width, Height, 0, static_cast<int>(PixelFormatType), static_cast<int>(DataType), NULL);
		}
		else
		{
			glTexImage2D(static_cast<int>(TextureImageTargetType), Level, static_cast<int>(InternalFormatType), Width, Height, 0, static_cast<int>(PixelFormatType), static_cast<int>(DataType), Buffers[0]);
		}
	}
	if (IsGenerateMipmap)
	{
		GenerateMipmap();
	}
}

FGLTexture::~FGLTexture()
{
	glDeleteTextures(1, &RendererID);
}

FGLTexture * FGLTexture::NewTexture2D(const int Width, const int Height, const GLPixelFormatType PixelFormatType, const void * Buffer)
{
	FGLTexture* GLTexture = new FGLTexture(GLTextureImageTargetType::Texture2D, 
											Width,
											Height,
											static_cast<GLInternalFormatType>(PixelFormatType),
											PixelFormatType, 
											0,
											GLDataType::UnsignedByte,
											true,
											{ Buffer });
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return GLTexture;
}

FGLTexture * FGLTexture::NewTextureCubeMap(int Width, int Height, const GLPixelFormatType PixelFormatType, const std::vector<const void*> Buffers)
{
	FGLTexture* GLTexture = new FGLTexture(GLTextureImageTargetType::TextureCubeMap,
											Width,
											Height,
											static_cast<GLInternalFormatType>(PixelFormatType),
											PixelFormatType,
											0,
											GLDataType::UnsignedByte,
											false,
											Buffers);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return GLTexture;
}

FGLTexture * FGLTexture::NewColorAttachmentTexture2D(const int Width, const int Height)
{
	FGLTexture* GLTexture = new FGLTexture(GLTextureImageTargetType::Texture2D,
											Width,
											Height,
											static_cast<GLInternalFormatType>(GLPixelFormatType::RGB),
											GLPixelFormatType::RGB,
											0,
											GLDataType::UnsignedByte,
											false,
											{ });
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return GLTexture;
}

FGLTexture * FGLTexture::NewDepthMapTexture2D(const int Width, const int Height)
{
	FGLTexture* GLTexture = new FGLTexture(GLTextureImageTargetType::Texture2D,
											Width,
											Height,
											static_cast<GLInternalFormatType>(GLPixelFormatType::DepthComponent),
											GLPixelFormatType::DepthComponent,
											0,
											GLDataType::Float,
											false,
											{ });
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	return GLTexture;
}

void FGLTexture::Bind(unsigned int Slot) const
{
	glActiveTexture(GL_TEXTURE0 + Slot);
	glBindTexture(GL_TEXTURE_2D, RendererID);
}

void FGLTexture::UnBind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

int FGLTexture::GetWidth() const
{
	return Width;
}

int FGLTexture::GetHeight() const
{
	return Height;
}

unsigned int FGLTexture::GetRendererID() const
{
	return RendererID;
}

GLTextureImageTargetType FGLTexture::GetTextureImageTargetType() const
{
	return TextureImageTargetType;
}

void FGLTexture::SetMinFilter(const GLTextureMinFilterType Type)
{
	glTexParameteri(static_cast<int>(TextureImageTargetType), GL_TEXTURE_MIN_FILTER, static_cast<int>(Type));
}

void FGLTexture::SetMagFilter(const GLTextureMagFilterType Type)
{
	glTexParameteri(static_cast<int>(TextureImageTargetType), GL_TEXTURE_MAG_FILTER, static_cast<int>(Type));
}

void FGLTexture::SetWrapS(const GLtextureWrapParameterType Type)
{
	glTexParameteri(static_cast<int>(TextureImageTargetType), GL_TEXTURE_WRAP_S, static_cast<int>(Type));
}

void FGLTexture::SetWrapT(const GLtextureWrapParameterType Type)
{
	glTexParameteri(static_cast<int>(TextureImageTargetType), GL_TEXTURE_WRAP_T, static_cast<int>(Type));
}

void FGLTexture::GenerateMipmap()
{
	glGenerateMipmap(static_cast<int>(TextureImageTargetType));
}
