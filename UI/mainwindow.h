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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QDebug>
#include <QFileDialog>
#include <QTimer>
#include <QThread>
#include <QPixmap>
#include <QAudioOutput>

#include "FVideoEditor.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public FVideoFrameReceiver
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    FPlayer* player = nullptr;
    FVideoDescription* videoDescription = nullptr;
    QVector<FImageTrack* > imageTracks;
    QVector<FAudioTrack* > audioTracks;

    FVideoProject* project = nullptr;

private:
    void cleanResource();

private slots:
    void on_actionOpenFile_triggered();

    void on_actionTestVideo_triggered();

    void on_playButton_clicked();

    void on_pauseButton_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_actionExport_triggered();

    void on_actionOpenProject_triggered();

private:
    Ui::MainWindow *ui;

    // FVideoFrameReceiver interface
public:
    void receiveFrame(const FImage* videoFrame) override;
};
#endif // MAINWINDOW_H
