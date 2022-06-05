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

#ifndef VideoEditor_Resolution_hpp
#define VideoEditor_Resolution_hpp

#include "Util.hpp"

namespace ks
{
	static const FSize KResolution480x640 = FSize(480, 640);
	static const FSize KResolution640x480 = FSize(640, 480);

	static const FSize KResolution800x600 = FSize(800, 600);
	static const FSize KResolution600x800 = FSize(600, 800);

	static const FSize KResolution720x1280 = FSize(720, 1280);
	static const FSize KResolution1280x720 = FSize(1280, 720);

	static const FSize KResolution1080x1920 = FSize(1080, 1920);
	static const FSize KResolution1920x1080 = FSize(1920, 1080);

	static const FSize KResolution2048x1080 = FSize(2048, 1080);
	static const FSize KResolution1998x1080 = FSize(1998, 1080);
	static const FSize KResolution2048x858 = FSize(2048, 858);
	static const FSize KResolution2560x1440 = FSize(2560, 1440);
	static const FSize KResolution2048x1536 = FSize(2048, 1536);
	static const FSize KResolution2560x1600 = FSize(2560, 1600);

	static const FSize KResolution4096x2160 = FSize(4096, 2160);
	static const FSize KResolution3840x2160 = FSize(3840, 2160);
}

#endif // VideoEditor_Resolution_hpp