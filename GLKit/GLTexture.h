#pragma once
#include <string>
#include <vector>
#include "ThirdParty/noncopyable.hpp"
#include "ThirdParty/opengl.h"

enum class GLTextureImageTargetType
{
	Texture2D = GL_TEXTURE_2D,
	TextureCubeMap = GL_TEXTURE_CUBE_MAP
};

enum class GLTextureMinFilterType
{
	Nearest = GL_NEAREST,
	Linear = GL_LINEAR,
	NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
	LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
	NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
	LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

enum class GLTextureMagFilterType
{
	Nearest = GL_NEAREST,
	Linear = GL_LINEAR
};

enum class GLtextureWrapParameterType
{
	ClampToEdge = GL_CLAMP_TO_EDGE,
	ClampToBorder = GL_CLAMP_TO_BORDER,
	MirroredRepeat = GL_MIRRORED_REPEAT,
	Repeat = GL_REPEAT
};

enum class GLInternalFormatType
{
	DepthComponent = GL_DEPTH_COMPONENT,
	RGB = GL_RGB,
	RGBA = GL_RGBA
};

enum class GLPixelFormatType
{
	DepthComponent = GL_DEPTH_COMPONENT,
	RGB = GL_RGB,
	RGBA = GL_RGBA
};

enum class GLDataType
{
	UnsignedByte = GL_UNSIGNED_BYTE,
	Float = GL_FLOAT,
};

class FGLTexture : public boost::noncopyable
{
private:
	unsigned int RendererID;

	int Width;

	int Height;

	int ColorChannels;

	GLTextureImageTargetType TextureImageTargetType;


	GLenum Format;

public:
	FGLTexture(const GLTextureImageTargetType TextureImageTargetType,
				const int Width, 
				const int Height, 
				const GLInternalFormatType InternalFormatType,
				const GLPixelFormatType PixelFormatType, 
				const int Level, 
				const GLDataType DataType,
				bool IsGenerateMipmap,
				const std::vector<const void*> Buffers);

	~FGLTexture();

	static FGLTexture* NewTexture2D(const int Width,
									const int Height, 
									const GLPixelFormatType PixelFormatType, 
									const void* Buffer);

	static FGLTexture* NewTextureCubeMap(int Width,
										int Height,
										const GLPixelFormatType PixelFormatType,
										const std::vector<const void*> Buffers);

	static FGLTexture* NewColorAttachmentTexture2D(const int Width, const int Height);

	static FGLTexture* NewDepthMapTexture2D(const int Width, const int Height);

	void Bind(unsigned int Slot) const;

	void UnBind();

	int GetWidth() const;

	int GetHeight() const;

	unsigned int GetRendererID() const;

	GLTextureImageTargetType GetTextureImageTargetType() const;

	void SetMinFilter(const GLTextureMinFilterType Type);

	void SetMagFilter(const GLTextureMagFilterType Type);

	void SetWrapS(const GLtextureWrapParameterType Type);

	void SetWrapT(const GLtextureWrapParameterType Type);

	void GenerateMipmap();
};