#include "ethread.h"

//thread

const int thread::NOT_EXITED = 0xefffffff;

thread::~thread()
{
	rejoin();
	delete &r_value;
	#ifdef OS_WIN32
	CloseHandle(_hthread);
	#endif
}

int thread::rejoin(int timeout)
{
	#ifdef OS_WIN32
	DWORD result;
	result = WaitForSingleObject(_hthread, timeout == 0 ? INFINITE : timeout);
	if (result == WAIT_FAILED)
		return -1;
	if (result == WAIT_TIMEOUT)
		return 0;
	if (result == WAIT_OBJECT_0)
		return 1;
	return -1;
	#else
	if (timeout > 0) {
        int retval, *pretval = &retval;
        timespec tout;
        tout.tv_sec = timeout;
        tout.tv_nsec = 0;
        if (pthread_timedjoin_np(threadid,(void**)&pretval,&tout) != 0)
		{
            if (errno == EBUSY or errno == ETIMEDOUT)
                return 0;
            return -1;
        }
    }
    else if (pthread_join(threadid,NULL) != 0)
        return -1;
    return 1;
	#endif
}

int thread::exitCode()
{
	#ifdef OS_WIN32
	DWORD exitCode;
	GetExitCodeThread(_hthread, &exitCode);
	return exitCode == STILL_ACTIVE ? NOT_EXITED : exitCode;
	#else
	int retval;
	int* pretval = &retval;
	if (pthread_tryjoin_np(threadid,(void**)&pretval) != 0)
		return NOT_EXITED;
	return retval;
	#endif
}

bool thread::running()
{
	return exitCode() == NOT_EXITED;
}

void sleep(int time)
{
	#ifdef OS_WIN32
	Sleep(time);
	#else
	usleep(time * 1000);
	#endif
}

//benaphore
#ifdef OS_WIN32
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
#endif

			