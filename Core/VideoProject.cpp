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
#include <QFile>
#include <QTextCodec>
#include <QFileInfo>
#include <QDebug>
#include "Vendor/rapidjson.h"

FVideoProject::FVideoProject(QString projectFilePath)
    : projectFilePath(projectFilePath)
{
    QFileInfo fileInfo(projectFilePath);
    projectDir = fileInfo.absolutePath();
    load();
}

FVideoProject::~FVideoProject()
{
    clean();
}

void FVideoProject::clean()
{
    if (videoDescription)
    {
        delete videoDescription;
        videoDescription = nullptr;
    }

    for (FImageTrack *imageTrack : imageTracks)
    {
        if (imageTrack)
        {
            delete imageTrack;
        }
    }
    imageTracks.clear();

    for (FAudioTrack *audioTrack : audioTracks)
    {
        if (audioTrack)
        {
            delete audioTrack;
        }
    }
    audioTracks.clear();
}

FMediaTimeRange converTimeRange(rapidjson::GenericObject<false, rapidjson::Value> timeRange, int timeScale = 600)
{
    double start = timeRange["start"].GetDouble();
    double end = timeRange["end"].GetDouble();
    return FMediaTimeRange(FMediaTime(start, timeScale), FMediaTime(end, timeScale));
}

void FVideoProject::load()
{
    clean();

    videoDescription = new FVideoDescription();

    std::ifstream ifs(projectFilePath.toStdString());
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    rapidjson::GenericArray<false, rapidjson::Value> videoTracksArray = doc["video_tracks"].GetArray();
    for (rapidjson::Value &item : videoTracksArray)
    {
        rapidjson::GenericObject<false, rapidjson::Value> videoTrackDes = item.GetObject();
        rapidjson::GenericObject<false, rapidjson::Value> sourceTimeRange = videoTrackDes["source_time_range"].GetObject();
        rapidjson::GenericObject<false, rapidjson::Value> targetTimeRange = videoTrackDes["target_time_range"].GetObject();

        QString filepath = projectDir;
        filepath.append("/");
        filepath.append(QString(videoTrackDes["path"].GetString()));

        qDebug() << filepath;

        FImageTrack *videoTrack = new FVideoTrack();
        videoTrack->filePath = filepath;
        videoTrack->timeMapping = FMediaTimeMapping(converTimeRange(sourceTimeRange), converTimeRange(targetTimeRange));
        videoDescription->imageTracks.append(videoTrack);
    }

    rapidjson::GenericArray<false, rapidjson::Value> audioTracksArray = doc["audio_tracks"].GetArray();
    for (rapidjson::Value &item : audioTracksArray)
    {
        rapidjson::GenericObject<false, rapidjson::Value> audioTrackDes = item.GetObject();
        rapidjson::GenericObject<false, rapidjson::Value> sourceTimeRange = audioTrackDes["source_time_range"].GetObject();
        rapidjson::GenericObject<false, rapidjson::Value> targetTimeRange = audioTrackDes["target_time_range"].GetObject();

        QString filepath = projectDir;
        filepath.append("/");
        filepath.append(QString(audioTrackDes["path"].GetString()));

        qDebug() << filepath;

        FAudioTrack *audioTrack = new FAudioTrack();
        audioTrack->filePath = filepath;
        audioTrack->timeMapping = FMediaTimeMapping(converTimeRange(sourceTimeRange), converTimeRange(targetTimeRange));
        videoDescription->audioTracks.append(audioTrack);
    }
}

const FVideoDescription *FVideoProject::getVideoDescription() const
{
    return videoDescription;
}

const QVector<FImageTrack *> FVideoProject::getImageTracks() const
{
    return imageTracks;
}

const QVector<FAudioTrack *> FVideoProject::getAudioTracks() const
{
    return audioTracks;
}

FImageTrack *FVideoProject::insertNewImageTrack()
{
    FImageTrack *videoTrack = new FVideoTrack();
    videoDescription->imageTracks.append(videoTrack);
}

FAudioTrack *FVideoProject::insertNewAudioTrack()
{
    FAudioTrack *audioTrack = new FAudioTrack();
    videoDescription->audioTracks.append(audioTrack);
}

bool FVideoProject::prepare()
{
    if (videoDescription)
    {
        videoDescription->prepare();
        return true;
    }
    else
    {
        return false;
    }
}
