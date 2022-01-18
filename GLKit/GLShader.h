#pragma once
#include <string>
#include <unordered_map>
#include "ThirdParty/glm.h"
#include "ThirdParty/noncopyable.hpp"
#include "GLTexture.h"

class FGLShader : public boost::noncopyable
{
private:
	std::unordered_map<std::string, int> UniformLocationCache;
	std::unordered_map<std::string, unsigned int> TextureSlotCache;

	static std::unordered_map<std::string, FGLShader*> Shaders;

protected:
	FGLShader();

public:
	~FGLShader();

	static unsigned int InvalidID;
	unsigned int RendererID = FGLShader::InvalidID;

	void Bind() const;
	void Unbind();

	static int CreateShader(const std::string& VertexShaderSource, const std::string& FragmentShaderSource);
	static unsigned int CompileShader(const unsigned int ShaderType, const std::string & ShaderSource);

	void SetUniform4f(const std::string& Name, float V0, float V1, float V2, float V3);
	void SetUniform4fv(const std::string& Name, const glm::vec4 &Value);
	void SetUniform3f(const std::string& Name, float V0, float V1, float V2);
	void SetUniform3fv(const std::string& Name, const glm::vec3 &Value);
	void SetUniform1i(const std::string & Name, int V0);
	void SetUniform1f(const std::string & Name, float V0);
	void SetUniformMat4(const std::string & Name, const glm::mat4& Matrix);

	void SetTexture(const std::string & Name, const FGLTexture & Texture);

	static FGLShader* NewWithSource(const std::string& VertexSourceCode, const std::string& FragSourceCode);
	static FGLShader* Cache(const std::string& Name);
	static FGLShader* NewOrCache(const std::string& VertexSourceCode, const std::string& FragSourceCode, const std::string& Name);
};