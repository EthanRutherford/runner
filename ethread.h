#ifndef ETHREAD_H
#define ETHREAD_H

#if defined(_MSC_VER) || defined(WIN32)  || defined(_WIN32) || defined(__WIN32__) \
	|| defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
	#define OS_WIN32
#endif

#ifdef OS_WIN32
	#include <Windows.h>
#else
	#include <pthread.h>
	#include <semaphore.h>
	#include <unistd.h>
#endif
#include <exception>
#include "safe_ptr.h"
#include "callable.h"

class thread_runtime_error : std::exception
{
	public:
		virtual const char* what() const throw()
		{ 
			return "A threading operation failed";
		}
};

template<class R, class... Args>
struct thread_func_info
{
	thread_func_info(func_ptr<R,Args...> cb, Args... a)
		: func(cb, a...) { r_value = new R;}
	void call() {
		*r_value = func.call();
	}
	callable<R,Args...> func;
	R* r_value;
};
template<class... Args>
struct thread_func_info<void, Args...>
{
	thread_func_info(func_ptr<void,Args...> cb, Args... a)
		: func(cb, a...) { r_value = new int; }
	void call() {
		func.call();
		*r_value = 0;
	}
	callable<void,Args...> func;
	int* r_value;
};

#ifdef OS_WIN32
template<class R, class... Args>
static DWORD WINAPI thread_routine(LPVOID param)
{
	thread_func_info<R,Args...>* info = 
		reinterpret_cast<thread_func_info<R,Args...>*>(param);
	info->call();
	delete info;
	return 0;
}
#else
template<class R, class... Args>
static void* thread_routine(void* param)
{
	thread_func_info<R,Args...>* info =
		reinterpret_cast<thread_func_info<R,Args...>*>(param);
	info->call();
	delete info;
	return 0;
}
#endif

class thread{
	public:
		template <class R, class... Args>
		thread(func_ptr<R,Args...> func, Args... args) {
			thread_func_info<R,Args...>* info = 
				new thread_func_info<R,Args...>(func, args...);
			r_value = info->r_value;
			#ifdef OS_WIN32
			_hthread = CreateThread(NULL,0,&thread_routine<R,Args...>,info,0,NULL);
			if (_hthread == INVALID_HANDLE_VALUE)
				throw thread_runtime_error();
			#else
			if (pthread_create(&threadid,NULL,&thread_routine<R,Args...>,info) != 0)
				throw thread_runtime_error();
			#endif
		}
		~thread();
		int rejoin(int timeout = 0);
		int exitCode();
		bool running();
		static void sleep(int time);
		template <class R>
		R getResult() {
			rejoin();
			return *(R*)r_value;
		}
		static const int NOT_EXITED;
	private:
		#ifdef OS_WIN32
		HANDLE _hthread;
		#else
		pthread_t threadid;
		#endif
		safe_ptr r_value;
};

class benaphore{
	public:
		benaphore();
		~benaphore();
		void lock();
		void unlock();
		void r_lock();
		void r_unlock();
	private:
		#ifdef OS_WIN32
		LONG counter;
		HANDLE semaphore;
		DWORD owner;
		#else
		long counter;
		sem_t semaphore;
		pthread_t owner;
		#endif
		int recursions;
};

#endif