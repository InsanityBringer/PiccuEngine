/*
* Descent 3: Piccu Engine
* Copyright (C) 2024 SaladBadger
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <chrono>
#include "ddio.h"

#ifdef WIN32
#include <Windows.h>
#include <timeapi.h>

TIMECAPS tc;
#endif

static std::chrono::steady_clock::duration basetick;

bool timer_Init(int preemptive, bool force_lores)
{
	basetick = std::chrono::steady_clock::now().time_since_epoch();
#ifdef WIN32
	//Wait, does this actually effect the QueryPerformanceCounter wrapped steady_clock?
	if (timeGetDevCaps(&tc, sizeof(tc)) == TIMERR_NOERROR)
		timeBeginPeriod(tc.wPeriodMin);
	else
	{
		tc.wPeriodMin = 1;
		timeBeginPeriod(1);
	}
#endif

	return true;
}

void timer_Close()
{
#ifdef WIN32
	timeEndPeriod(tc.wPeriodMin);
#endif
}

//	returns time in seconds
float timer_GetTime()
{
	return (float)timer_GetTime64();
}

double timer_GetTime64()
{
	auto time = std::chrono::steady_clock::now().time_since_epoch() - basetick;
	return (static_cast<longlong>(std::chrono::duration_cast<std::chrono::microseconds>(time).count()) / 1000000.0);
}

//returns time in milliseconds
longlong timer_GetMSTime()
{
	auto time = std::chrono::steady_clock::now().time_since_epoch() - basetick;
	return static_cast<longlong>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
}
