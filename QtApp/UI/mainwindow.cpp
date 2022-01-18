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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Video Editor");
}

MainWindow::~MainWindow()
{
    cleanResource();
    delete ui;
    ui = nullptr;
}

void MainWindow::cleanResource()
{
    if (player)
    {
        delete player;
        player = nullptr;
    }
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

    if (project)
    {
        delete project;
        project = nullptr;
    }
}

void MainWindow::on_actionOpenFile_triggered()
{
    cleanResource();
    QString filePath = QFileDialog::getOpenFileName(this, "open video file", R"(./)", "Image files(*.mp4 *.mov *.flv *.mkv)");
    if (filePath.isEmpty())
    {
        return;
    }
    player = new FPlayer();
    videoDescription = new FVideoDescription();
    player->videoDescription = videoDescription;

    {
        FImageTrack *videoTrack = new FVideoTrack();
        videoTrack->filePath = filePath;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(100.0, 600));
        videoTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        imageTracks.append(videoTrack);
    }

    {
        FAudioTrack *audioTrack = new FAudioTrack();
        audioTrack->filePath = filePath;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(100.0, 600));
        audioTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        audioTracks.append(audioTrack);
    }
    
    videoDescription->audioTracks = audioTracks;
    videoDescription->imageTracks = imageTracks;
    videoDescription->prepare();

    player->setReceiver(this);
    player->play();
}

void MainWindow::receiveFrame(const FImage *videoFrame)
{
    if (videoFrame)
    {
        const QImage *image = videoFrame->image();
        if (image)
        {
            if (image->isNull() == false)
            {
                ui->playerView->setPixmap(QPixmap::fromImage(*image));
            }
            else
            {
                qDebug("image is null");
            }
        }
        else
        {
            qDebug("image is null");
        }
    }
    else
    {
        qDebug("image is null");
    }
}

void MainWindow::on_actionTestVideo_triggered()
{
    cleanResource();
    player = new FPlayer();
    videoDescription = new FVideoDescription();
    player->videoDescription = videoDescription;

    QString file0 = QCoreApplication::applicationDirPath().append("/Mat").append("/ElephantsDream.mp4");
    QString file1 = QCoreApplication::applicationDirPath().append("/Mat").append("/BigBuckBunny.mp4");

    {
        FImageTrack* videoTrack = new FVideoTrack();
        videoTrack->filePath = file0;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(100.0, 600));
        videoTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        imageTracks.append(videoTrack);
    }

    {
        FImageTrack *videoTrack = new FVideoTrack();
        videoTrack->filePath = file1;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(100.0, 600));
        videoTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        imageTracks.append(videoTrack);
    }

    {
        FAudioTrack *audioTrack = new FAudioTrack();
        audioTrack->filePath = file0;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(100.0, 600));
        audioTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        audioTracks.append(audioTrack);
    }

    {
        FAudioTrack *audioTrack = new FAudioTrack();
        audioTrack->filePath = file1;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(100.0, 600));
        audioTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        audioTracks.append(audioTrack);
    }

    videoDescription->audioTracks = audioTracks;
    videoDescription->imageTracks = imageTracks;
    videoDescription->prepare();

    player->setReceiver(this);
    player->play();
}

void MainWindow::on_playButton_clicked()
{
    if (player)
    {
        player->play();
    }
}

void MainWindow::on_pauseButton_clicked()
{
    if (player)
    {
        player->pause();
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    if (player)
    {
        FMediaTime time = FMediaTime((double)value / (double)ui->horizontalSlider->maximum() * 100.0, 600);
        player->seek(time, [&] {

        });
    }
}


void MainWindow::on_actionExport_triggered()
{
    FVideoDescription* videoDescription = new FVideoDescription();
    videoDescription->renderSize = KResolution1280x720;

    videoDescription->fps = 24.0;

    QString file0 = QCoreApplication::applicationDirPath().append("/Mat").append("/ElephantsDream.mp4");
    QString file1 = QCoreApplication::applicationDirPath().append("/Mat").append("/BigBuckBunny.mp4");

    {
        FImageTrack* videoTrack = new FVideoTrack();
        videoTrack->filePath = file0;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(10.0, 600));
        videoTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        videoDescription->imageTracks.append(videoTrack);
    }

    {
        FImageTrack *videoTrack = new FVideoTrack();
        videoTrack->filePath = file1;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(10.0, 600));
        videoTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        videoDescription->imageTracks.append(videoTrack);
    }

    {
        FAudioTrack *audioTrack = new FAudioTrack();
        audioTrack->filePath = file0;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(10.0, 600));
        audioTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        videoDescription->audioTracks.append(audioTrack);
    }

    {
        FAudioTrack *audioTrack = new FAudioTrack();
        audioTrack->filePath = file1;
        FMediaTimeRange targetTimeRange = FMediaTimeRange(FMediaTime(0.0, 600), FMediaTime(10.0, 600));
        audioTrack->timeMapping = FMediaTimeMapping(targetTimeRange, targetTimeRange);
        videoDescription->audioTracks.append(audioTrack);
    }

    videoDescription->prepare();

    FExportSession* exportSession = new FExportSession(videoDescription);

    exportSession->start([&videoDescription, &exportSession](int ret){
        qDebug() << ret << "Finish.";

        for (auto item : videoDescription->imageTracks)
        {
            delete item;
        }
        videoDescription->imageTracks.clear();

        for (auto item : videoDescription->audioTracks)
        {
            delete item;
        }
        videoDescription->audioTracks.clear();

        delete exportSession;
        exportSession = nullptr;

        delete videoDescription;
        videoDescription = nullptr;
    });
}

void MainWindow::on_actionOpenProject_triggered()
{
    cleanResource();
    QString filePath = QFileDialog::getOpenFileName(this, "Open project", QString(), "(*.veproj)");
    if (filePath.isEmpty())
    {
        return;
    }
    player = new FPlayer();
    project = new FVideoProject(filePath);
    project->prepare();
    player->videoDescription = project->getVideoDescription();
    player->setReceiver(this);
    player->play();
}
