// Copyright (C) 2021 lmc
// 
// This file is part of VideoEditor.
// 
// VideoEditor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
// 
// VideoEditor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VideoEditor.  If not, see <http://www.gnu.org/licenses/>.

#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>

extern "C"
{
#include "SDL.h"
}

#include "Core/FVideoEditor.h"
#include "spdlog/spdlog.h"
#include "GLKit.h"

#include "imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


GLFWwindow* Window = nullptr;
FVideoProject* videoProject = nullptr;
FImagePlayer* imagePlayer = nullptr;
FImageCompositionPipeline* imageCompositionPipeline = nullptr;
float currentTime = 0;
bool isPause = false;
FAudioPlayer* audioPlayer = nullptr;

void glInit()
{
	const bool initRet = glfwInit();
	assert(initRet);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Window = glfwCreateWindow(1280, 720, "Video Editor", NULL, NULL);
	assert(Window);

	glfwSwapInterval(1);

	glfwMakeContextCurrent(Window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glEnable(GL_MULTISAMPLE);
}

void imguiInit()
{
	const std::string Version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(Window, true);
	const bool initRet = ImGui_ImplOpenGL3_Init(Version.c_str());
	assert(initRet);
}

void imguiDestroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void imguiDrawUI()
{
	ImGuiWindowFlags Flags = 0;
	Flags |= ImGuiWindowFlags_MenuBar;

	ImGui::SetNextWindowBgAlpha(0.2);
	ImGui::Begin("VideoEditor", nullptr, Flags);

	assert(imagePlayer != nullptr);

	if (ImGui::SliderFloat("current time", &currentTime, 0.0, 100.0))
	{
		const FMediaTime seekTime = FMediaTime(currentTime, 600);

		imagePlayer->seek(seekTime);
		if (audioPlayer)
		{
			audioPlayer->seek(seekTime);
		}
	}

	if (ImGui::Button("play"))
	{
		isPause = false;
		imagePlayer->play();
		if (audioPlayer)
		{
			audioPlayer->play();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("pause"))
	{
		isPause = true;
		imagePlayer->pause();
		if (audioPlayer)
		{
			audioPlayer->pause();
		}
	}

	if (isPause)
	{
		ImGui::Text("pause");
	}
	else
	{
		ImGui::Text("playing");
	}

	ImGui::End();
}

void imaguiTick()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	imguiDrawUI();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void sdlAudioCallback(void *userdata, Uint8 * stream, int len)
{
	if (audioPlayer)
	{
		audioPlayer->getPCMBuffer([&stream, &len](const FAudioPCMBuffer *buffer) {
			if (buffer)
			{
				const float* data = buffer->immutableFloatChannelData()[0];
				memcpy(stream, data, len);
			}
			else
			{
				SDL_memset(stream, 0, len);
			}
		});
	}
	else
	{
		SDL_memset(stream, 0, len);
	}
}

void initSDLAudio()
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
	{
		assert(false);
	}
	SDL_AudioSpec audioSpec;
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_F32LSB;
	audioSpec.channels = 2;
	audioSpec.silence = 0;
	audioSpec.samples = 1024;
	audioSpec.callback = sdlAudioCallback;

	SDL_AudioSpec actualAudioSpec;

	if (SDL_OpenAudio(&audioSpec, nullptr) < 0)
	{
		assert(false);
	}

	SDL_PauseAudio(0);
}

struct Vertex
{
	glm::vec2 iPosition;
	glm::vec2 iTexCoords;
};

int main(int argc, char *argv[])
{
	spdlog::set_level(spdlog::level::trace);
	assert(argc > 1);
	const std::string projectFilePath = argv[1];

	imageCompositionPipeline = new FImageCompositionPipeline();
	videoProject = new FVideoProject(projectFilePath);
	assert(videoProject->getVideoDescription()->renderContext.audioRenderContext.audioFormat.isNonInterleaved() == false);
	bool ret = videoProject->prepare();
	imagePlayer = new FImagePlayer();
	imagePlayer->pipeline = imageCompositionPipeline;
	imagePlayer->replace(videoProject->getVideoDescription());
	audioPlayer = new FAudioPlayer();
	audioPlayer->replace(videoProject->getVideoDescription());

	glInit();
	imguiInit();


	std::vector<Vertex> vertexBuffer;

	Vertex topLeft;
	Vertex topRight;
	Vertex bottomLeft;
	Vertex bottomRight;

	topLeft.iPosition = glm::vec2(-1.0, 1.0);
	topLeft.iTexCoords = glm::vec2(0, 0);

	topRight.iPosition = glm::vec2(1.0, 1.0);
	topRight.iTexCoords = glm::vec2(1, 0);

	bottomLeft.iPosition = glm::vec2(-1.0, -1.0);
	bottomLeft.iTexCoords = glm::vec2(0, 1);

	bottomRight.iPosition = glm::vec2(1.0, -1.0);
	bottomRight.iTexCoords = glm::vec2(1, 1);


	vertexBuffer.push_back(topLeft);
	vertexBuffer.push_back(topRight);
	vertexBuffer.push_back(bottomLeft);
	vertexBuffer.push_back(bottomRight);

	std::vector<unsigned int> indexBufferData = { 0, 1, 2, 1, 2, 3 };

	FGLVertexObject* glVO = new FGLVertexObject(vertexBuffer.data(), sizeof(Vertex) * vertexBuffer.size(), indexBufferData.data(), indexBufferData.size(), []()
	{
		FGLVertexBufferLayout layout;
		layout.Float(2).Float(2);
		return  layout;
	});

	const std::string vert =
		R"(
			#version 330 core

			layout (location = 0) in vec2 iPosition;
			layout (location = 1) in vec2 iTexCoords;

			out vec2 oTexCoords;

			void main()
			{
				oTexCoords = iTexCoords;
				gl_Position = vec4(iPosition.x, iPosition.y, 0.0, 1.0);
			}
		)";

	const std::string frag =
		R"(
			#version 330 core

			uniform sampler2D imageTexture;
			in vec2 oTexCoords;
			out vec4 oFragColor;

			void main()
			{
				oFragColor = texture(imageTexture, oTexCoords);
			}
		)";

	FGLShader* imageShader = FGLShader::NewWithSource(vert, frag);

	glVO->Bind();
	imageShader->Bind();

	initSDLAudio();

	while (!glfwWindowShouldClose(Window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		currentTime = imagePlayer->getCurrentTime().seconds();

		const FPixelBuffer *const pixelBuffer = imagePlayer->getPixelBuffer();
		if (pixelBuffer)
		{
			FGLTexture* texture = FGLTexture::NewTexture2D(pixelBuffer->width(), pixelBuffer->height(), GLPixelFormatType::RGBA, pixelBuffer->immutableData()[0]);
			imageShader->SetTexture("imageTexture", *texture);
			glDrawElements(GL_TRIANGLES, indexBufferData.size(), GL_UNSIGNED_INT, 0);
			delete texture;
		}

		imaguiTick();
		glfwSwapBuffers(Window);
		glfwPollEvents();
	}
	imguiDestroy();
	glfwTerminate();

	return 0;
}