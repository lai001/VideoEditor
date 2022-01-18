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

#ifndef FSIMPLETIMER_H
#define FSIMPLETIMER_H

#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

#include "Vendor/noncopyable.hpp"

class FSimpleTimer :public boost::noncopyable
{

public:
	FSimpleTimer(const long long timeInterval, std::function<void(FSimpleTimer&)> closure);
	~FSimpleTimer();

	void fire();
	void invalidate();

	long long getDuration() const;

	void setIsPause(bool flag);
	bool getIsPause() const;
	bool getIsValid() const;

private:
	std::function<void(FSimpleTimer&)> closure;
	const long long timeInterval;

	long long getCurrentClockTime() const;
	long long lastClockTime = 0;
	long long duration = 0;

	void setDuration(long long value);

	bool isPause = false;

	bool isValid = true;

	bool token = false;

	mutable std::mutex lock;
};


#endif // !FSIMPLETIMER_H