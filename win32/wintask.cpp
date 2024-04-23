/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
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

//	Allow Full DD system access.
#include "DDAccess.h"

//	Library includes
#include "TaskSystem.h"
#include "pserror.h"

//	Standard includes
#include <windows.h>


//	---------------------------------------------------------------------------
//	osTask implementation
//	---------------------------------------------------------------------------


osTask::osTask(unsigned (*func)(void *), tTaskPriority priority, void *parm)
{
	int pri;

	task_os_handle = 0;
	task_os_id = 0;

 	task_os_handle = (unsigned)CreateThread(NULL, 0,
								(LPTHREAD_START_ROUTINE)func, 
								(LPVOID)parm, 0, 
								(LPDWORD)&task_os_id);
	if (task_os_handle == 0) {
		Int3();									// Get Samir.
	}
 
	if (priority == TASKPRIORITY_HIGHEST)
		pri = THREAD_PRIORITY_TIME_CRITICAL;
	else if (priority == TASKPRIORITY_NORMAL)
		pri = THREAD_PRIORITY_NORMAL;
	else if (priority == TASKPRIORITY_LOWEST)
		pri = THREAD_PRIORITY_LOWEST;

	SetThreadPriority((HANDLE)task_os_handle, pri);
}


osTask::~osTask()
{
	if (task_os_handle) {
		CloseHandle((HANDLE)task_os_handle);
	}
}


bool osTask::error() const
{
	if (!task_os_handle) return 1;
	else return 0;
}


void osTask::suspend()							// suspends task
{
	if (task_os_handle) {
		SuspendThread((HANDLE)task_os_handle);
	}
}


void osTask::resume()							// resumes task
{
	if (task_os_handle) {
		ResumeThread((HANDLE)task_os_handle);
	}
}


//	---------------------------------------------------------------------------
//	osEvent implementation
//	---------------------------------------------------------------------------
osEvent::osEvent(char *name)
{
	event_os_handle = (unsigned)CreateEvent(NULL, TRUE, FALSE, name);	
}


osEvent::~osEvent()
{
	if (event_os_handle)
		CloseHandle((HANDLE)event_os_handle);
}

// signal the event so blocking can stop
void osEvent::signal()
{
	if (event_os_handle) 
		SetEvent((HANDLE)event_os_handle);
}


// clear the event so blocking can continue
void osEvent::clear()
{
	if (event_os_handle)
		ResetEvent((HANDLE)event_os_handle);
}


// block until signaled
bool osEvent::block(int timeout)
{
	DWORD res;

	if (event_os_handle) {
		res = WaitForSingleObject((HANDLE)event_os_handle, (timeout==-1) ? INFINITE : timeout);
		if (res == WAIT_OBJECT_0) return 1;
		else if (res == WAIT_ABANDONED) return 0;
	}

	return 0;
}


bool osEvent::error() const
{
	if (!event_os_handle) return 1;
	else return 0;
}



//	This establishes a mutual exclusion object.  once one thread locks the object, 
//	any code in another thread that contains a mutex check will block or skip that code until
//	the locking thread unlocks it.
osMutex::osMutex()
{
	mutex_os_handle = 0;
}


osMutex::~osMutex()
{
	Destroy();
}

bool osMutex::Create()								// creates a mutex object.
{
	if (!mutex_os_handle) {
		mutex_os_handle = (unsigned)CreateMutex(NULL, FALSE, NULL);
		return mutex_os_handle ? true : false;
	}

	return false;
}

void osMutex::Destroy()								// destroys a mutex object
{
	if (mutex_os_handle) {
		CloseHandle((HANDLE)mutex_os_handle);
		mutex_os_handle = 0;
	}
}

	
// calling thread will attempt to acquire mutex (wait until timeout) if timeout == -1, wait forever...
bool osMutex::Acquire(int timeout)
{
	DWORD res;

	if (!mutex_os_handle) 
		return true;							// we should return true because the caller will skip code if false is returned.

	if (timeout == -1) 
		timeout = INFINITE;

	res = WaitForSingleObject((HANDLE)mutex_os_handle, timeout);
	if (res == WAIT_OBJECT_0 || res == WAIT_ABANDONED) 
		return true;
	else 
		return false;
}


// calling thread releases control of mutex.
void osMutex::Release()
{
	if (mutex_os_handle) {
		ReleaseMutex((HANDLE)mutex_os_handle);
	}
}




