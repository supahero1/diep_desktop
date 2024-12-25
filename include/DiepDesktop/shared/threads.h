#pragma once

#include <stdint.h>


#ifdef _WIN32
	#include <windows.h>

	typedef HANDLE ThreadID;
	typedef CRITICAL_SECTION Mutex;
	typedef CONDITION_VARIABLE CondVar;
#else
	#include <pthread.h>

	typedef pthread_t ThreadID;
	typedef pthread_mutex_t Mutex;
	typedef pthread_cond_t CondVar;
#endif


typedef void
(*ThreadType)(
	void* Arg
	);


extern void
ThreadInit(
	ThreadID* Thread,
	ThreadType Func,
	void* Arg
	);


extern void
ThreadWait(
	ThreadID Thread
	);


extern void
ThreadQuit(
	void
	);


extern void
ThreadDestroy(
	ThreadID Thread
	);


extern void
MutexInit(
	Mutex* Mtx
	);


extern void
MutexDestroy(
	Mutex* Mtx
	);


extern void
MutexLock(
	Mutex* Mtx
	);


extern void
MutexUnlock(
	Mutex* Mtx
	);


extern void
CondVarInit(
	CondVar* Var
	);


extern void
CondVarDestroy(
	CondVar* Var
	);


extern void
CondVarWait(
	CondVar* Var,
	Mutex* Mtx
	);


extern void
CondVarWake(
	CondVar* Var
	);
