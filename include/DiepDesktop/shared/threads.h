#pragma once

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>


typedef pthread_mutex_t Mutex;


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


typedef pthread_cond_t CondVar;


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


typedef sem_t Semaphore;


extern void
SemaphoreInit(
	Semaphore* Sem,
	uint32_t Value
	);


extern void
SemaphoreDestroy(
	Semaphore* Sem
	);


extern void
SemaphoreWait(
	Semaphore* Sem
	);


extern void
SemaphoreTimedWait(
	Semaphore* Sem,
	uint64_t Nanoseconds
	);


extern void
SemaphorePost(
	Semaphore* Sem
	);


typedef pthread_t ThreadT;

typedef void
(*ThreadFunc)(
	void* Data
	);

typedef struct ThreadData
{
	ThreadFunc Func;
	void* Arg;
}
ThreadData;


extern void
ThreadInit(
	ThreadT* Thread,
	ThreadData Data
	);


extern void
ThreadDestroy(
	ThreadT* Thread
	);


extern void
ThreadCancelOn(
	void
	);


extern void
ThreadCancelOff(
	void
	);


extern void
ThreadAsyncOn(
	void
	);


extern void
ThreadAsyncOff(
	void
	);


extern void
ThreadDetach(
	ThreadT Thread
	);


extern void
ThreadJoin(
	ThreadT Thread
	);


extern void
ThreadCancelSync(
	ThreadT Thread
	);


extern void
ThreadCancelAsync(
	ThreadT Thread
	);


extern void
ThreadExit(
	void
	);


extern void
ThreadSleep(
	uint64_t Nanoseconds
	);


typedef struct ThreadsT
{
	ThreadT* Threads;
	uint32_t Used;
	uint32_t Size;
}
ThreadsT;


extern void
ThreadsInit(
	ThreadsT* Threads
	);


extern void
ThreadsDestroy(
	ThreadsT* Threads
	);


extern void
ThreadsAdd(
	ThreadsT* Threads,
	ThreadData Data,
	uint32_t Count
	);


extern void
ThreadsCancelSync(
	ThreadsT* Threads,
	uint32_t Count
	);


extern void
ThreadsCancelAsync(
	ThreadsT* Threads,
	uint32_t Count
	);


extern void
ThreadsShutdownSync(
	ThreadsT* Threads
	);


extern void
ThreadsShutdownAsync(
	ThreadsT* Threads
	);


typedef struct ThreadPoolT
{
	Semaphore Sem;
	Mutex Mtx;

	ThreadData* Queue;
	uint32_t Used;
	uint32_t Size;
}
ThreadPoolT;


extern void
ThreadPoolFunc(
	void* Data
	);


extern void
ThreadPoolInit(
	ThreadPoolT* Pool
	);


extern void
ThreadPoolDestroy(
	ThreadPoolT* Pool
	);


extern void
ThreadPoolLock(
	ThreadPoolT* Pool
	);


extern void
ThreadPoolUnlock(
	ThreadPoolT* Pool
	);


extern void
ThreadPoolAddU(
	ThreadPoolT* Pool,
	ThreadData Data
	);


extern void
ThreadPoolAdd(
	ThreadPoolT* Pool,
	ThreadData Data
	);


extern void
ThreadPoolTryWorkU(
	ThreadPoolT* Pool
	);


extern void
ThreadPoolTryWork(
	ThreadPoolT* Pool
	);


extern void
ThreadPoolWorkU(
	ThreadPoolT* Pool
	);


extern void
ThreadPoolWork(
	ThreadPoolT* Pool
	);
