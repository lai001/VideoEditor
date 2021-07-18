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

#ifndef FTHREAD_H
#define FTHREAD_H

#include <functional>
#include <QObject>
#include <QThread>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);

signals:
    void resultReady();

public slots:
    void job();

public:
    std::function<void()> closure;

};

class FThread : public QObject
{
    Q_OBJECT
public:
    explicit FThread(QObject *parent = nullptr);
    ~FThread();

    void start();
    std::function<void()> finishClosure;

signals:
    void startRunning();

public slots:
    void onReceivResult();

private:
    QThread workThread;
    Worker *worker;

public:
    void async(std::function<void()> closure, std::function<void()> finishClosure);
};

#endif // FTHREAD_H
