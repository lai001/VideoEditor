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

#include "Core/FVideoEditor.h"
#include "spdlog/spdlog.h"

inline bool isFileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int main(int argc, char *argv[])
{
	spdlog::set_level(spdlog::level::debug);
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

		assert(isFileExists(projectFilePath));
		assert(outputFilePath.length() > 0);

		FVideoProject* _videoProject = new FVideoProject(projectFilePath);
		FVideoProject& videoProject = *_videoProject;

		bool ret = videoProject.prepare();
		const FVideoDescription *des = videoProject.getVideoDescription();
		FImageCompositionPipeline pipeline;

		FExportSession session = FExportSession(*des, pipeline);


		session.start(outputFilePath, [](const std::string& type, const FMediaTime& time) 
		{
			if (type == "video")
			{
				spdlog::debug("encode video frame {}", time.seconds());
			}
			else if (type == "audio")
			{
				spdlog::debug("encode audio frame {}", time.seconds());
			}
		});
	}
	catch (TCLAP::ArgException &e)
	{
		spdlog::error("{} for arg ", e.error(), e.argId());
	}
	return 0;
}