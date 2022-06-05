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
#include <memory>
#include <assert.h>

extern "C"
{
#include "SDL.h"
}

#include <VideoEditor/VideoEditor.hpp>
#include <Foundation/Foundation.hpp>
#include <KSImage/KSImage.hpp>
#include <spdlog/spdlog.h>
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Platform/WindowsPlatform.hpp"

ks::FVideoProject* videoProject = nullptr;
ks::FImagePlayer* imagePlayer = nullptr;
ks::FImageCompositionPipeline* imageCompositionPipeline = nullptr;
float currentTime = 0;
bool isPause = false;
ks::FAudioPlayer* audioPlayer = nullptr;
static std::unique_ptr<WindowsPlatform> windowsPlatformPtr;
unsigned int currentWindowWidth = 1280;
unsigned int currentWindowHeight = 720;

std::unique_ptr<ks::IRenderEngine> defaultRenderEngine = std::unique_ptr<ks::IRenderEngine>();
std::unique_ptr<ks::IRenderEngine> filterRenderEngine = std::unique_ptr<ks::IRenderEngine>();

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ImGuiInit()
{
	windowsPlatformPtr->WndProcCallback = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& intercept)
	{
		intercept = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return true;
	};
	windowsPlatformPtr->sizeChange = [](unsigned int width, unsigned int height)
	{
		currentWindowWidth = width;
		currentWindowHeight = height;
	};
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowsPlatformPtr->getHwnd());
	ImGui_ImplDX11_Init(windowsPlatformPtr->getDevice(), windowsPlatformPtr->getDeviceContext());
}

void ImGuiDestroy()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void imguiDrawUI()
{
	ImGuiWindowFlags Flags = 0;
	Flags |= ImGuiWindowFlags_MenuBar;

	ImGui::SetNextWindowBgAlpha(0.2);
	ImGui::Begin("VideoEditor", nullptr, Flags);

	assert(imagePlayer != nullptr);

	if (ImGui::SliderFloat("Current time", &currentTime, 0.0, 100.0))
	{
		const ks::MediaTime seekTime = ks::MediaTime(currentTime, 600);

		imagePlayer->seek(seekTime);
		if (audioPlayer)
		{
			audioPlayer->seek(seekTime);
		}
	}

	if (ImGui::Button("Play"))
	{
		isPause = false;
		imagePlayer->play();
		if (audioPlayer)
		{
			audioPlayer->play();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause"))
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
		ImGui::Text("Pause");
	}
	else
	{
		ImGui::Text("Playing");
	}

	ImGui::End();
}

void imaguiTick()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	imguiDrawUI();
	ImGui::Render();
	windowsPlatformPtr->setRenderTarget();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void sdlAudioCallback(void *userdata, Uint8 * stream, int len)
{
	if (audioPlayer == nullptr)
	{
		SDL_memset(stream, 0, len);
		return;
	}

	audioPlayer->getPCMBuffer([&stream, &len](const ks::AudioPCMBuffer *buffer) {
		if (buffer == nullptr)
		{
			SDL_memset(stream, 0, len);
			return;
		}
		memcpy(stream, buffer->immutableFloatChannelData()[0], len);
	});
}

void initSDLAudio(const int samples)
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
	audioSpec.samples = samples;
	audioSpec.callback = sdlAudioCallback;

	SDL_AudioSpec actualAudioSpec;

	if (SDL_OpenAudio(&audioSpec, nullptr) < 0)
	{
		assert(false);
	}

	SDL_PauseAudio(0);
}

void drawResult(const ks::PixelBuffer* pixelBuffer)
{
	windowsPlatformPtr->setRenderTarget();

	static const std::string vert = ks::File::read(ks::Application::getResourcePath("Shader/vert.hlsl"), nullptr);
	static const std::string frag = ks::File::read(ks::Application::getResourcePath("Shader/frag.hlsl"), nullptr);

	ks::IRenderEngine& engine = *defaultRenderEngine.get();

	struct Vertex
	{
		glm::vec2 aPosition;
		glm::vec2 texCoord;
	};

	std::vector<unsigned int> indexBufferData = { 0, 1, 2, 2, 1, 3 };

	std::vector<Vertex> vertexBuffer;
	{
		Vertex topLeft;
		Vertex topRight;
		Vertex bottomLeft;
		Vertex bottomRight;

		topLeft.aPosition = glm::vec2(-1.0, 1.0);
		topLeft.texCoord = glm::vec2(0.0, 0.0);
		topRight.aPosition = glm::vec2(1.0, 1.0);
		topRight.texCoord = glm::vec2(1.0, 0.0);
		bottomLeft.aPosition = glm::vec2(-1.0, -1.0);
		bottomLeft.texCoord = glm::vec2(0.0, 1.0);
		bottomRight.aPosition = glm::vec2(1.0, -1.0);
		bottomRight.texCoord = glm::vec2(1.0, 1.0);

		vertexBuffer.push_back(topLeft);
		vertexBuffer.push_back(topRight);
		vertexBuffer.push_back(bottomLeft);
		vertexBuffer.push_back(bottomRight);
	}

	ks::ITexture2D* colorMap = engine.createTexture2D(pixelBuffer->getWidth(),
		pixelBuffer->getHeight(),
		ks::TextureFormat::R8G8B8A8_UNORM,
		pixelBuffer->getImmutableData()[0]);
	static ks::IShader* shader = engine.createShader(vert, frag);
	shader->setTexture2D("colorMap", *colorMap);
	ks::IRenderBuffer * renderBuffer = engine.createRenderBuffer(vertexBuffer.data(), vertexBuffer.size(), sizeof(Vertex),
		*shader,
		indexBufferData.data(), indexBufferData.size(), ks::IIndexBuffer::IndexDataType::uint32);
	ks::IBlendState* blendState = engine.createBlendState(ks::BlendStateDescription::Addition::getDefault(), ks::BlendStateDescription::getDefault());
	ks::IDepthStencilState* depthStencilState = engine.createDepthStencilState(ks::DepthStencilStateDescription::getDefault());
	ks::IRasterizerState* rasterizerState = engine.createRasterizerState(ks::RasterizerStateDescription::getDefault());
	renderBuffer->setViewport(0, 0, currentWindowWidth, currentWindowHeight);
	renderBuffer->setBlendState(*blendState);
	renderBuffer->setDepthStencilState(*depthStencilState);
	renderBuffer->setRasterizerState(*rasterizerState);
	renderBuffer->setPrimitiveTopologyType(ks::PrimitiveTopologyType::trianglelist);
	renderBuffer->commit(nullptr);

	engine.erase(renderBuffer);
	engine.erase(blendState);
	engine.erase(depthStencilState);
	engine.erase(rasterizerState);
	engine.erase(colorMap);
}

int main(int argc, char *argv[])
{
	assert(argc > 1);

	spdlog::set_level(spdlog::level::trace);
	ks::Application::Init(argc, argv);
	const int audioSamples = 1024;
	const std::string projectFilePath = argv[1];

	WindowsPlatform::Configuration cfg;
	cfg.windowHeight = currentWindowHeight;
	cfg.windowWidth = currentWindowWidth;
	cfg.windowName = "VideoEditor";
	windowsPlatformPtr = std::make_unique<WindowsPlatform>(cfg);

	ks::D3D11RenderEngineCreateInfo createInfo;
	ks::D3D11RenderEngineCreateInfo::NativeData nativeData;
	nativeData.device = windowsPlatformPtr->getDevice();
	nativeData.context = windowsPlatformPtr->getDeviceContext();
	createInfo.data = &nativeData;
	defaultRenderEngine = std::unique_ptr<ks::IRenderEngine>(ks::RenderEngine::create(createInfo));
	filterRenderEngine = std::unique_ptr<ks::IRenderEngine>(ks::RenderEngine::create(createInfo));
	ks::InitVideoEditor(filterRenderEngine.get());

	initSDLAudio(audioSamples);
	ImGuiInit();
	defer
	{
		ImGuiDestroy();
	};

	imageCompositionPipeline = new ks::FImageCompositionPipeline();
	videoProject = new ks::FVideoProject(projectFilePath);
	assert(videoProject->getVideoDescription()->renderContext.audioRenderContext.audioFormat.isNonInterleaved() == false);
	bool ret = videoProject->prepare();
	imagePlayer = new ks::FImagePlayer();
	imagePlayer->pipeline = imageCompositionPipeline;
	imagePlayer->replace(videoProject->getVideoDescription());
	audioPlayer = new ks::FAudioPlayer(audioSamples);
	audioPlayer->replace(videoProject->getVideoDescription());

	while (windowsPlatformPtr->shouldClose() == false)
	{
		windowsPlatformPtr->clearColor();
		currentTime = imagePlayer->getCurrentTime().seconds();

		if (const ks::PixelBuffer * pixelBuffer = imagePlayer->getPixelBuffer())
		{
			drawResult(pixelBuffer);
		}
		imaguiTick();
		windowsPlatformPtr->present();
	};

	return 0;
}