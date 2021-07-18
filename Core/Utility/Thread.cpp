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

#include "Thread.h"
#include <QDebug>

FThread::FThread(QObject *parent) : QObject(parent)
{
    worker = new Worker();
    worker->moveToThread(&workThread);

    connect(this, &FThread::startRunning, worker, &Worker::job);
    connect(&workThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &Worker::resultReady, this, &FThread::onReceivResult);

    workThread.start();
}

FThread::~FThread()
{
    workThread.quit();
    workThread.wait();
}

void FThread::start()
{
    emit startRunning();
}

void FThread::onReceivResult()
{
    finishClosure();
}

void FThread::async(std::function<void()> closure, std::function<void()> finishClosure)
{
    worker->closure = closure;
    this->finishClosure = finishClosure;
    startRunning();
}

Worker::Worker(QObject *parent) : QObject(parent)
{
}

void Worker::job()
{
    closure();

    emit resultReady();
}
