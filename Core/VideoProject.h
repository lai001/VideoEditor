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

#ifndef VIDEOPROJECT_H
#define VIDEOPROJECT_H

#include <QString>
#include "FVideoEditor.h"

class FVideoProject
{
private:
    QString projectDir;
    QString projectFilePath;

    FVideoDescription *videoDescription = nullptr;
    QVector<FImageTrack *> imageTracks;
    QVector<FAudioTrack *> audioTracks;

    void clean();

public:
    FVideoProject(QString projectFilePath);
    ~FVideoProject();

    void load();
    bool prepare();

    const FVideoDescription *getVideoDescription() const;
    const QVector<FImageTrack *> getImageTracks() const;
    const QVector<FAudioTrack *> getAudioTracks() const;

    FImageTrack *insertNewImageTrack();
    FAudioTrack *insertNewAudioTrack();
};

#endif // VIDEOPROJECT_H