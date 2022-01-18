#include "GLShader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "ThirdParty/opengl.h"

std::unordered_map<std::string, FGLShader*> FGLShader::Shaders;
unsigned int FGLShader::InvalidID = -1;

FGLShader::FGLShader()
{
}

FGLShader::~FGLShader()
{
	if (RendererID != FGLShader::InvalidID)
	{
		glDeleteProgram(RendererID);
	}
}

void FGLShader::Bind() const
{
	assert(RendererID != FGLShader::InvalidID);
	glUseProgram(RendererID);
}

void FGLShader::Unbind()
{
	glUseProgram(0);
}

int FGLShader::CreateShader(const std::string& VertexShader, const std::string& FragmentShader)
{
	unsigned int ProgramID = glCreateProgram();
	try
	{
		unsigned int Vs = CompileShader(GL_VERTEX_SHADER, VertexShader);
		unsigned int Fs = CompileShader(GL_FRAGMENT_SHADER, FragmentShader);
		glAttachShader(ProgramID, Vs);
		glAttachShader(ProgramID, Fs);
		glLinkProgram(ProgramID);
		glValidateProgram(ProgramID);
		glDeleteShader(Vs);
		glDeleteShader(Fs);
		return ProgramID;
	}
	catch (const std::exception E)
	{
		glDeleteProgram(ProgramID);
		throw E;
	}
}

unsigned int FGLShader::CompileShader(const unsigned int ShaderType, const std::string & ShaderSource)
{
	unsigned int Shader = glCreateShader(ShaderType);
	const char* Src = ShaderSource.c_str();
	glShaderSource(Shader, 1, &Src, nullptr);
	glCompileShader(Shader);

	int Result;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Result);

	if (Result == false)
	{
		int Length;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &Length);
		char* Message = (char*)alloca(Length * sizeof(char));
		glGetShaderInfoLog(Shader, Length, &Length, Message);
		glDeleteShader(Shader);
		throw std::runtime_error(std::string(ShaderType == GL_VERTEX_SHADER ? " vertex" : " fragment") + std::string(" shader compile fail.") + std::string(Message));
	}

	return Shader;
}

void FGLShader::SetUniform4f(const std::string& Name, float V0, float V1, float V2, float V3)
{
	if (UniformLocationCache.find(Name) == UniformLocationCache.end())
	{
		UniformLocationCache[Name] = glGetUniformLocation(RendererID, Name.c_str());
	}
	glUniform4f(UniformLocationCache[Name], V0, V1, V2, V3);
}

void FGLShader::SetUniform4fv(const std::string & Name, const glm::vec4 & Value)
{
	if (UniformLocationCache.find(Name) == UniformLocationCache.end())
	{
		UniformLocationCache[Name] = glGetUniformLocation(RendererID, Name.c_str());
	}
	glUniform4fv(UniformLocationCache[Name], 1, &Value[0]);
}

void FGLShader::SetUniform3f(const std::string & Name, float V0, float V1, float V2)
{
	if (UniformLocationCache.find(Name) == UniformLocationCache.end())
	{
		UniformLocationCache[Name] = glGetUniformLocation(RendererID, Name.c_str());
	}
	glUniform3f(UniformLocationCache[Name], V0, V1, V2);
}

void FGLShader::SetUniform3fv(const std::string & Name, const glm::vec3 & Value)
{
	if (UniformLocationCache.find(Name) == UniformLocationCache.end())
	{
		UniformLocationCache[Name] = glGetUniformLocation(RendererID, Name.c_str());
	}
	glUniform3fv(UniformLocationCache[Name], 1, &Value[0]);
}

void FGLShader::SetUniform1i(const std::string & Name, int V0)
{
	if (UniformLocationCache.find(Name) == UniformLocationCache.end())
	{
		UniformLocationCache[Name] = glGetUniformLocation(RendererID, Name.c_str());
	}
	glUniform1i(UniformLocationCache[Name], V0);
}

void FGLShader::SetUniform1f(const std::string & Name, float V0)
{
	if (UniformLocationCache.find(Name) == UniformLocationCache.end())
	{
		UniformLocationCache[Name] = glGetUniformLocation(RendererID, Name.c_str());
	}
	glUniform1f(UniformLocationCache[Name], V0);
}

void FGLShader::SetUniformMat4(const std::string & Name, const glm::mat4& Matrix)
{
	if (UniformLocationCache.find(Name) == UniformLocationCache.end())
	{
		UniformLocationCache[Name] = glGetUniformLocation(RendererID, Name.c_str());
	}
	glUniformMatrix4fv(UniformLocationCache[Name], 1, GL_FALSE, &Matrix[0][0]);
}

void FGLShader::SetTexture(const std::string & Name, const FGLTexture& Texture)
{
	unsigned int Slot;
	if (TextureSlotCache.find(Name) == TextureSlotCache.end())
	{
		std::vector<unsigned int> values;
		values.reserve(TextureSlotCache.size());

		auto maxValue = std::max_element(values.begin(), values.end());
		if (maxValue == values.end())
		{
			Slot = 0;
		}
		else
		{
			Slot = *maxValue + 1;
		}
		TextureSlotCache[Name] = Slot;
	}
	else
	{
		Slot = TextureSlotCache[Name];
	}

	Texture.Bind(Slot);
	SetUniform1i(Name, Slot);
}

FGLShader * FGLShader::NewWithSource(const std::string & VertexSourceCode, const std::string & FragSourceCode)
{
	try
	{
		int ID = FGLShader::CreateShader(VertexSourceCode, FragSourceCode);
		FGLShader* Shader = new FGLShader();
		Shader->RendererID = ID;
		return Shader;
	}
	catch (const std::exception E)
	{
		return nullptr;
	}
}

FGLShader * FGLShader::Cache(const std::string & Name)
{
	FGLShader* Shader = FGLShader::Shaders[Name];
	return Shader;
}

FGLShader * FGLShader::NewOrCache(const std::string & VertexSourceCode, const std::string & FragSourceCode, const std::string & Name)
{
	FGLShader* Shader = Cache(Name);
	if (Shader == nullptr)
	{
		Shader = NewWithSource(VertexSourceCode, FragSourceCode);
		if (Shader)
		{
			FGLShader::Shaders[Name] = Shader;
		}
	}
	return Shader;
}

