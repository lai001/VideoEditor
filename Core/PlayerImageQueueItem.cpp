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

#include "PlayerImageQueueItem.h"

FPlayerImageQueueItem::FPlayerImageQueueItem(QSize size, QImage::Format format)
{
    _qimage = new QImage(size, format);
    _fimage = new FImage();
    _fimage->fillImage(_qimage);
}

FPlayerImageQueueItem::~FPlayerImageQueueItem()
{
    if (_fimage)
    {
        delete _fimage;
    }
    _fimage = nullptr;
    _qimage = nullptr;
}

QImage *FPlayerImageQueueItem::qimage() const
{
    if (_qimage)
    {
        return _qimage;
    }
    return nullptr;
}

const FImage *FPlayerImageQueueItem::fimage() const
{
    if (_fimage)
    {
        return _fimage;
    }
    return nullptr;
}


