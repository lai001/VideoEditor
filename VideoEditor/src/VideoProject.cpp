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

#include "VideoProject.hpp"
#include <fstream>
#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "Util.hpp"

namespace ks
{
	VideoProject::VideoProject(const std::string& projectFilePath)
		: projectFilePath(projectFilePath)
	{
		projectDir = getFolder(projectFilePath);

		videoDescription = new VideoDescription();

		const std::string rawString = ks::File::read(projectFilePath, nullptr);
		const Json j3 = Json::parse(rawString);
		const Json video_render_context = j3.at("video_render_context");
		const Json audio_render_context = j3.at("audio_render_context");
		const Json video_tracks = j3.at("video_tracks");
		const Json audio_tracks = j3.at("audio_tracks");

		loadVideoTracks(video_tracks);
		loadAudioTracks(audio_tracks);

		loadVideoRenderContext(video_render_context, videoDescription->renderContext.videoRenderContext);
		loadAudioRenderContext(audio_render_context, videoDescription->renderContext.audioRenderContext);
	}

	VideoProject::~VideoProject()
	{
		clean();
	}

	void VideoProject::clean()
	{
		for (IImageTrack *imageTrack : videoDescription->imageTracks)
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

	bool VideoProject::loadVideoRenderContext(const Json & json, VideoRenderContext& context)
	{
		assert(json.at("render_size_width").is_null() == false);
		assert(json.at("render_size_height").is_null() == false);
		assert(json.at("fps").is_null() == false);
		assert(json.at("render_scale").is_null() == false);
		int width = json.at("render_size_width");
		int height = json.at("render_size_height");
		context.fps = json.at("fps");
		context.renderScale = json.at("render_scale");
		context.renderSize = FSize(width, height);
		context.format = PixelBuffer::FormatType::rgba8;
		return true;
	}

	bool VideoProject::loadAudioRenderContext(const Json & json, AudioRenderContext& context)
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

		const Json audio_format = json.at("audio_format");

		float sample_rate = audio_format.at("sample_rate");
		std::string sample_type = audio_format.at("sample_type");
		int channel = audio_format.at("channel");
		bool is_noninterleaved = audio_format.at("is_noninterleaved");

		assert(table.find(sample_type) != table.end());
		AudioSampleType sampleType = table[sample_type];
		assert(table1.find(sampleType) != table1.end());

		context.audioFormat.formatType = AudioFormatIdentifiersType::pcm;
		context.audioFormat.channelsPerFrame = channel;
		context.audioFormat.framesPerPacket = 1;
		context.audioFormat.formatFlags = ks::AudioFormatFlag();
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
			context.audioFormat.formatFlags = context.audioFormat.formatFlags.insert(AudioFormatFlag::isNonInterleaved);
		}

		if ((sampleType == AudioSampleType::sint16) || (sampleType == AudioSampleType::sint32))
		{
			context.audioFormat.formatFlags = context.audioFormat.formatFlags.insert(AudioFormatFlag::isSignedInteger);
		}
		else if ((sampleType == AudioSampleType::float32) || (sampleType == AudioSampleType::float64))
		{
			context.audioFormat.formatFlags = context.audioFormat.formatFlags.insert(AudioFormatFlag::isFloat);
		}

		return true;
	}

	bool VideoProject::loadVideoTracks(const Json & json)
	{
		for (size_t i = 0; i < json.size(); i++)
		{
			const Json videoTrackJson = json.at(i);
			const std::string path = videoTrackJson.at("path");
			const Json source_time_range = videoTrackJson.at("source_time_range");
			const Json target_time_range = videoTrackJson.at("target_time_range");
			const Json rectJson = videoTrackJson.at("rect");
			const Rect rect = converRect(rectJson);
			const std::string filepath = projectDir + "/" + path;
			VideoTrack *videoTrack = new VideoTrack();
			videoTrack->rect = rect;
			videoTrack->filePath = filepath;
			videoTrack->timeMapping = MediaTimeMapping(converTimeRange(source_time_range, 600), converTimeRange(target_time_range, 600));
			videoDescription->imageTracks.push_back(videoTrack);
		}
		return true;
	}

	bool VideoProject::loadAudioTracks(const Json & json)
	{
		for (size_t i = 0; i < json.size(); i++)
		{
			const Json videoTrackJson = json.at(i);
			const std::string path = videoTrackJson.at("path");
			const Json source_time_range = videoTrackJson.at("source_time_range");
			const Json target_time_range = videoTrackJson.at("target_time_range");
			const std::string filepath = projectDir + "/" + path;
			FAudioTrack *audioTrack = new FAudioTrack();
			audioTrack->filePath = filepath;
			audioTrack->timeMapping = MediaTimeMapping(converTimeRange(source_time_range, 44100), converTimeRange(target_time_range, 44100));
			videoDescription->audioTracks.push_back(audioTrack);
		}
		return true;
	}

	MediaTimeRange VideoProject::converTimeRange(const Json & json, int timeScale)
	{
		assert(json.at("start").is_null() == false);
		assert(json.at("end").is_null() == false);
		double start = json.at("start");
		double end = json.at("end");
		return MediaTimeRange(MediaTime(start, timeScale), MediaTime(end, timeScale));
	}

	Rect VideoProject::converRect(const Json & json)
	{
		assert(json.at("x").is_null() == false);
		assert(json.at("y").is_null() == false);
		assert(json.at("width").is_null() == false);
		assert(json.at("height").is_null() == false);
		float x = json.at("x");
		float y = json.at("y");
		float width = json.at("width");
		float height = json.at("height");
		return Rect(x, y, width, height);
	}

	const VideoDescription *VideoProject::getVideoDescription() const
	{
		return videoDescription;
	}

	const std::vector<IImageTrack *> VideoProject::getImageTracks() const
	{
		return videoDescription->imageTracks;
	}

	const std::vector<FAudioTrack *> VideoProject::getAudioTracks() const
	{
		return videoDescription->audioTracks;
	}

	IImageTrack *VideoProject::insertNewImageTrack()
	{
		IImageTrack *videoTrack = new VideoTrack();
		videoDescription->imageTracks.push_back(videoTrack);
		return videoTrack;
	}

	FAudioTrack *VideoProject::insertNewAudioTrack()
	{
		FAudioTrack *audioTrack = new FAudioTrack();
		videoDescription->audioTracks.push_back(audioTrack);
		return audioTrack;
	}

	bool VideoProject::prepare()
	{
		videoDescription->prepare();
		return true;
	}
}