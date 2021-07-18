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

#include "Util.h"

#include "FFmpeg.h"
#include <QApplication>
#include <QAbstractEventDispatcher>

template <typename F>
static void postToObject(F &&fun, QObject *obj = qApp) {
  QMetaObject::invokeMethod(obj, std::forward<F>(fun));
}

template <typename F>
static void postToThread(F && fun, QThread *thread = qApp->thread()) {
   auto *obj = QAbstractEventDispatcher::instance(thread);
   Q_ASSERT(obj);
   QMetaObject::invokeMethod(obj, std::forward<F>(fun));
}

FUtil::FUtil()
{

}

std::function<double ()> FUtil::measue(bool fps)
{
    auto begin = av_gettime();
    return [begin, fps]{
        double duration = av_gettime() - begin;
        if (fps)
        {
            return 1000000.0 / duration;
        }
        return duration;
    };
}

void FUtil::runInMainThread(std::function<void()> closure)
{
    postToThread([closure]{
        closure();
    });
}

QString FUtil::ffmpegErrorDescription(int errorCode)
{
    char errStr[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(errorCode, errStr, sizeof(errStr));
    return QString(errStr);
}

