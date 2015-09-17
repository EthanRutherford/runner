#include "ethread.h"

//thread

const int thread::NOT_EXITED = 0xefffffff;

thread::~thread()
{
	rejoin();
	delete &r_value;
	CloseHandle(_hthread);
}

int thread::rejoin(int timeout)
{
	DWORD result;
	result = WaitForSingleObject(_hthread, timeout == 0 ? INFINITE : timeout);
	if (result == WAIT_FAILED)
		return -1;
	if (result == WAIT_TIMEOUT)
		return 0;
	if (result == WAIT_OBJECT_0)
		return 1;
	return -1;
}

int thread::exitCode()
{
	DWORD exitCode;
	GetExitCodeThread(_hthread, &exitCode);
	return exitCode == STILL_ACTIVE ? NOT_EXITED : exitCode;
}

bool thread::running()
{
	return exitCode() == NOT_EXITED;
}

//benaphore

benaphore::benaphore()
{
	owner = 0;
	recursions = 0;
	counter = 0;
	semaphore = CreateSemaphore(NULL, 0, 1, NULL);
}

benaphore::~benaphore()
{
	CloseHandle(semaphore);
}

void benaphore::lock()
{
	if (InterlockedIncrement(&counter) > 1)
		WaitForSingleObject(semaphore, INFINITE);
}

void benaphore::unlock()
{
	if (InterlockedDecrement(&counter) > 0)
		ReleaseSemaphore(semaphore, 1, NULL);
}

void benaphore::r_lock()
{
	DWORD tid = GetCurrentThreadId();
	if (InterlockedIncrement(&counter) > 1)
		if (tid != owner)
			WaitForSingleObject(semaphore, INFINITE);
	owner = tid;
	recursions++;
}

void benaphore::r_unlock()
{
	DWORD tid = GetCurrentThreadId();
	//LIGHT_ASSERT(tid == owner);
	if (--recursions == 0)
		owner = 0;
	if (InterlockedDecrement(&counter) > 0)
		if (recursions == 0)
			ReleaseSemaphore(semaphore, 1, NULL);
}

			