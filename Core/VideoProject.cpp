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

#include "VideoProject.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "Utility/FUtility.h"

FVideoProject::FVideoProject(const std::string& projectFilePath)
	: projectFilePath(projectFilePath)
{
	projectDir = FUtil::GetFolder(projectFilePath);

	videoDescription = new FVideoDescription();

	std::ifstream ifs(projectFilePath);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);

	loadVideoTracks(doc);
	loadAudioTracks(doc);

	loadVideoRenderContext(doc, videoDescription->renderContext.videoRenderContext);
	loadAudioRenderContext(doc, videoDescription->renderContext.audioRenderContext);

	const FAudioFormat audioFormat = videoDescription->renderContext.audioRenderContext.audioFormat;
	//assert(FUtil::getAVSampleFormat(audioFormat) == AV_SAMPLE_FMT_FLT);
}

FVideoProject::~FVideoProject()
{
	clean();
}

void FVideoProject::clean()
{
	for (FImageTrack *imageTrack : videoDescription->imageTracks)
	{
		delete imageTrack;
	}
	videoDescription->imageTracks.clear();

	for (FAudioTrack *audioTrack : videoDescription->audioTracks)
	{
		delete audioTrack;
	}
	videoDescription->audioTracks.clear();
	delete videoDescription;
}

bool FVideoProject::loadVideoRenderContext(rapidjson::Document& doc, FVideoRenderContext& context)
{
	rapidjson::GenericObject<false, rapidjson::Value> video_render_context = doc["video_render_context"].GetObject();

	int width = video_render_context["render_size_width"].GetInt();
	int height = video_render_context["render_size_height"].GetInt();
	float fps = video_render_context["fps"].GetFloat();
	float renderScale = video_render_context["render_scale"].GetFloat();

	context.fps = fps;
	context.renderScale = renderScale;
	context.renderSize = FSize(width, height);
	context.format = PixelBufferFormatType::rgba32;

	return true;
}

bool FVideoProject::loadAudioRenderContext(rapidjson::Document & doc, FAudioRenderContext& context)
{
	std::unordered_map<std::string, AudioSampleType> table;
	//table["uint16"] = AudioSampleType::uint16;
	//table["uint32"] = AudioSampleType::uint32;
	table["sint16"] = AudioSampleType::sint16;
	table["sint32"] = AudioSampleType::sint32;
	table["float32"] = AudioSampleType::float32;
	table["float64"] = AudioSampleType::float64;

	std::unordered_map<AudioSampleType, unsigned int> table1;
	//table1[AudioSampleType::uint16] = 2;
	//table1[AudioSampleType::uint32] = 4;
	table1[AudioSampleType::sint16] = 2;
	table1[AudioSampleType::sint32] = 4;
	table1[AudioSampleType::float32] = 4;
	table1[AudioSampleType::float64] = 8;

	rapidjson::GenericObject<false, rapidjson::Value> audio_render_context = doc["audio_render_context"].GetObject();
	rapidjson::GenericObject<false, rapidjson::Value> audio_format = audio_render_context["audio_format"].GetObject();

	float sample_rate = audio_format["sample_rate"].GetFloat();
	std::string sample_type = audio_format["sample_type"].GetString();
	int channel = audio_format["channel"].GetInt();
	bool is_noninterleaved = audio_format["is_noninterleaved"].GetBool();
	AudioSampleType sampleType = table[sample_type];

	context.audioFormat.formatType = AudioFormatIdentifiersType::pcm;
	context.audioFormat.channelsPerFrame = channel;
	context.audioFormat.framesPerPacket = 1;
	context.audioFormat.formatFlags = 0;
	context.audioFormat.bitsPerChannel = table1[sampleType] * 8;

	if (is_noninterleaved)
	{
		context.audioFormat.bytesPerFrame = table1[sampleType];
	}
	else
	{
		context.audioFormat.bytesPerFrame = table1[sampleType] * channel;
	}
	context.audioFormat.bytesPerPacket = context.audioFormat.bytesPerFrame;

	if (is_noninterleaved)
	{
		context.audioFormat.formatFlags = Bitmask::insert(context.audioFormat.formatFlags, AudioFormatFlag::isNonInterleaved);
	}

	if ((sampleType == AudioSampleType::sint16) || (sampleType == AudioSampleType::sint32))
	{
		context.audioFormat.formatFlags = Bitmask::insert(context.audioFormat.formatFlags, AudioFormatFlag::isSignedInteger);
	}
	else if ((sampleType == AudioSampleType::float32) || (sampleType == AudioSampleType::float64))
	{
		context.audioFormat.formatFlags = Bitmask::insert(context.audioFormat.formatFlags, AudioFormatFlag::isFloat);
	}


	return true;
}

bool FVideoProject::loadVideoTracks(rapidjson::Document& doc)
{
	rapidjson::GenericArray<false, rapidjson::Value> videoTracksArray = doc["video_tracks"].GetArray();
	for (rapidjson::Value &item : videoTracksArray)
	{
		rapidjson::GenericObject<false, rapidjson::Value> videoTrackDes = item.GetObject();
		rapidjson::GenericObject<false, rapidjson::Value> sourceTimeRange = videoTrackDes["source_time_range"].GetObject();
		rapidjson::GenericObject<false, rapidjson::Value> targetTimeRange = videoTrackDes["target_time_range"].GetObject();

		std::string filepath = projectDir;
		filepath.append("/");
		filepath.append(std::string(videoTrackDes["path"].GetString()));

		FVideoTrack *videoTrack = new FVideoTrack();
		videoTrack->filePath = filepath;
		videoTrack->timeMapping = FMediaTimeMapping(converTimeRange(sourceTimeRange, 600), converTimeRange(targetTimeRange, 600));
		videoDescription->imageTracks.push_back(videoTrack);
	}
	return true;
}

bool FVideoProject::loadAudioTracks(rapidjson::Document& doc)
{
	rapidjson::GenericArray<false, rapidjson::Value> audioTracksArray = doc["audio_tracks"].GetArray();
	for (rapidjson::Value &item : audioTracksArray)
	{
		rapidjson::GenericObject<false, rapidjson::Value> audioTrackDes = item.GetObject();
		rapidjson::GenericObject<false, rapidjson::Value> sourceTimeRange = audioTrackDes["source_time_range"].GetObject();
		rapidjson::GenericObject<false, rapidjson::Value> targetTimeRange = audioTrackDes["target_time_range"].GetObject();

		std::string filepath = projectDir;
		filepath.append("/");
		filepath.append(std::string(audioTrackDes["path"].GetString()));

		FAudioTrack *audioTrack = new FAudioTrack();
		audioTrack->filePath = filepath;
		audioTrack->timeMapping = FMediaTimeMapping(converTimeRange(sourceTimeRange, 44100), converTimeRange(targetTimeRange, 44100));
		videoDescription->audioTracks.push_back(audioTrack);
	}
	return true;
}

FMediaTimeRange FVideoProject::converTimeRange(rapidjson::GenericObject<false, rapidjson::Value> timeRange, int timeScale)
{
	double start = timeRange["start"].GetDouble();
	double end = timeRange["end"].GetDouble();
	return FMediaTimeRange(FMediaTime(start, timeScale), FMediaTime(end, timeScale));
}

const FVideoDescription *FVideoProject::getVideoDescription() const
{
	return videoDescription;
}

const std::vector<FImageTrack *> FVideoProject::getImageTracks() const
{
	return videoDescription->imageTracks;
}

const std::vector<FAudioTrack *> FVideoProject::getAudioTracks() const
{
	return videoDescription->audioTracks;
}

FImageTrack *FVideoProject::insertNewImageTrack()
{
	FImageTrack *videoTrack = new FVideoTrack();
	videoDescription->imageTracks.push_back(videoTrack);
	return videoTrack;
}

FAudioTrack *FVideoProject::insertNewAudioTrack()
{
	FAudioTrack *audioTrack = new FAudioTrack();
	videoDescription->audioTracks.push_back(audioTrack);
	return audioTrack;
}

bool FVideoProject::prepare()
{
	videoDescription->prepare();
	return true;
}
