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

#include "VideoFrame.h"

FVideoFrame::FVideoFrame()
{
    _image = new FImage();
}

FVideoFrame::~FVideoFrame()
{
    if (_image)
    {
        delete _image;
    }
    _image = nullptr;
}

uchar *FVideoFrame::fillImage(QSize size, QImage::Format format)
{
    if (_image)
    {
        return _image->fillImage(size, format);
    }
    return nullptr;
}

uchar *FVideoFrame::fillImage(int width, int height, QImage::Format format)
{
    if (_image)
    {
        return _image->fillImage(width, height, format);
    }
    return nullptr;
}

const FImage *FVideoFrame::image() const
{
    return _image;
}
