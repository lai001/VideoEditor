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

#include <sys/stat.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <tclap/CmdLine.h>
#include <Foundation/Foundation.hpp>
#include <VideoEditor/VideoEditor.hpp>
#include "spdlog/spdlog.h"
#include "Platform/WindowsPlatform.hpp"

static std::unique_ptr<WindowsPlatform> windowsPlatformPtr;

int main(int argc, char *argv[])
{
	ks::Application::Init(argc, argv);
	spdlog::set_level(spdlog::level::debug);

	WindowsPlatform::Configuration cfg;
	cfg.showWindowCommandType = WindowsPlatform::ShowWindowCommandType::hide;
	windowsPlatformPtr = std::make_unique<WindowsPlatform>(cfg);

	ks::D3D11RenderEngineCreateInfo createInfo;
	ks::D3D11RenderEngineCreateInfo::NativeData nativeData;
	nativeData.device = windowsPlatformPtr->getDevice();
	nativeData.context = windowsPlatformPtr->getDeviceContext();
	createInfo.data = &nativeData;
	static auto filterRenderEngine = std::unique_ptr<ks::IRenderEngine>(ks::RenderEngine::create(createInfo));
	ks::InitVideoEditor(filterRenderEngine.get());

	try
	{
		TCLAP::CmdLine cmd("VideoEditor", ' ', "0.1.0");
		TCLAP::ValueArg<std::string> nameArg("i", "project_file_path", "project file path", false, "", "string");
		TCLAP::ValueArg<std::string> nameArg1("o", "output_file_path", "output file path", false, "", "string");
		cmd.add(nameArg);
		cmd.add(nameArg1);
		cmd.parse(argc, argv);
		
		const std::string projectFilePath = nameArg.getValue();
		const std::string outputFilePath = nameArg1.getValue();
		
		assert(ks::File::isReadable(projectFilePath));
		assert(outputFilePath.length() > 0);

		std::unique_ptr<ks::FVideoProject> videoProject = std::unique_ptr<ks::FVideoProject>(new ks::FVideoProject(projectFilePath));

		bool ret = videoProject->prepare();
		const ks::FVideoDescription *des = videoProject->getVideoDescription();
		ks::FImageCompositionPipeline pipeline;

		ks::FExportSession session = ks::FExportSession(*des, pipeline);

		session.start(outputFilePath, [](const ks::FExportSession::EncodeType& type, const ks::MediaTime& time)
		{
			if (type == ks::FExportSession::EncodeType::video)
			{
				spdlog::debug("encode video frame {}", time.seconds());
			}
			else if (type == ks::FExportSession::EncodeType::audio)
			{
				spdlog::debug("encode audio frame {}", time.seconds());
			}
		});
		spdlog::info(ks::Application::getAppDir() + "/" + outputFilePath);
		std::cin >> std::string();
	}
	catch (TCLAP::ArgException &e)
	{
		spdlog::error("{} for arg ", e.error(), e.argId());
		std::cin >> std::string();
	}
	return 0;
}