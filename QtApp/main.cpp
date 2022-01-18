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

#include <QApplication>
#include <QTextCodec>
#include <QtDebug>
#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>

#include "Test.h"

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageOutput);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFileInfo fileInfo(context.file);
    QDateTime time = QDateTime::currentDateTime();
    QString strTime = time.toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString level;
    switch (type)
    {
    case QtDebugMsg:
        level = QString(" [Debug] ");
        break;
    case QtInfoMsg:
        level = QString(" [Info] ");
        break;
    case QtWarningMsg:
        level = QString(" [Warning] ");
        break;
    case QtCriticalMsg:
        level = QString(" [Critical] ");
        break;
    case QtFatalMsg:
        level = QString(" [Fatal] ");
        break;
    default:
        level = QString(" [Err]");
        break;
    }

    QString strMessage = QString("\033[34m%1 %2 %3 %4 %5 \n\033[0m  -> %6\n").arg(level).arg(strTime).arg(fileInfo.fileName()).arg(context.line).arg(context.function).arg(msg);

    FILE *device = stdout;
    if (type > QtInfoMsg)
    {
        device = stderr;
    }

    QTextStream outputStream(device);
    outputStream.setCodec(QTextCodec::codecForName("System"));
    outputStream << strMessage.toLocal8Bit();
    outputStream.flush();
}
