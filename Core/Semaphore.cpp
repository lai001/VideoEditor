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

#include "Semaphore.h"

FSemaphore::FSemaphore(int count)
	:count(count)
{
}

void FSemaphore::signal()
{
	std::unique_lock<std::mutex> lockg(lock);
	++count;
	cv.notify_one();
}

void FSemaphore::wait()
{
	std::unique_lock<std::mutex> lockg(lock);
	cv.wait(lockg, [=] { return count > 0; });
	--count;
}
