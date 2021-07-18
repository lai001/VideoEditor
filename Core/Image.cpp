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

#include "Image.h"

FImage::FImage()
{
}

FImage::~FImage()
{
    if (_image)
    {
        delete _image;
        _image = nullptr;
    }
}

uchar *FImage::fillImage(const QSize &size, QImage::Format format)
{
    return fillImage(size.width(), size.height(), format);
}

uchar *FImage::fillImage(int width, int height, QImage::Format format)
{
    if (_image)
    {
        delete _image;
        _image = nullptr;
    }
    _image = new QImage(width, height, format);
    extent = QRect(0, 0, width, height);
    return _image->bits();
}

uchar *FImage::fillImage(QImage *image)
{
    _image = image;
}

const QImage *FImage::image() const
{
    return _image;
}
