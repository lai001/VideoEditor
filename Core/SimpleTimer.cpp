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

#include "SimpleTimer.h"
#include "ThirdParty/spdlog.h"
#include <assert.h>

FSimpleTimer::FSimpleTimer(const long long timeInterval, std::function<void(FSimpleTimer&)> closure)
	: closure(closure), timeInterval(timeInterval)
{
}

FSimpleTimer::~FSimpleTimer()
{

}

void FSimpleTimer::fire()
{
	assert(token == false);
	token = true;
	std::thread([this]()
	{
		lastClockTime = getCurrentClockTime();
		while (getIsValid())
		{
			const long long now = getCurrentClockTime();
			setDuration(now - lastClockTime);
			lastClockTime = now;
			if (getIsPause() == false)
			{
				closure(*this);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(timeInterval));
		}
	}).detach();
}

void FSimpleTimer::invalidate()
{
	std::lock_guard<std::mutex> lockg(lock);
	isValid = false;
}

long long FSimpleTimer::getDuration() const
{
	std::lock_guard<std::mutex> lockg(lock);
	return duration;
}

void FSimpleTimer::setIsPause(bool flag)
{
	std::lock_guard<std::mutex> lockg(lock);
	isPause = flag;
}

bool FSimpleTimer::getIsPause() const
{
	std::lock_guard<std::mutex> lockg(lock);
	return isPause;
}

bool FSimpleTimer::getIsValid() const
{
	std::lock_guard<std::mutex> lockg(lock);
	return isValid;
}

long long FSimpleTimer::getCurrentClockTime() const
{
	std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

void FSimpleTimer::setDuration(long long value)
{
	std::lock_guard<std::mutex> lockg(lock);
	duration = value;
}
