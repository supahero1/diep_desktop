#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdatomic.h>


void
MutexInit(
	Mutex* Mtx
	)
{
	int Status = pthread_mutex_init(Mtx, NULL);
	HardenedAssertEQ(Status, 0);
}


void
MutexDestroy(
	Mutex* Mtx
	)
{
	int Status = pthread_mutex_destroy(Mtx);
	HardenedAssertEQ(Status, 0);
}


void
MutexLock(
	Mutex* Mtx
	)
{
	int Status = pthread_mutex_lock(Mtx);
	AssertEQ(Status, 0);
}


void
MutexUnlock(
	Mutex* Mtx
	)
{
	int Status = pthread_mutex_unlock(Mtx);
	AssertEQ(Status, 0);
}


void
CondVarInit(
	CondVar* Var
	)
{
	int Status = pthread_cond_init(Var, NULL);
	HardenedAssertEQ(Status, 0);
}


void
CondVarDestroy(
	CondVar* Var
	)
{
	int Status = pthread_cond_destroy(Var);
	HardenedAssertEQ(Status, 0);
}


void
CondVarWait(
	CondVar* Var,
	Mutex* Mtx
	)
{
	int Status = pthread_cond_wait(Var, Mtx);
	AssertEQ(Status, 0);
}


void
CondVarWake(
	CondVar* Var
	)
{
	int Status = pthread_cond_signal(Var);
	AssertEQ(Status, 0);
}


void
SemaphoreInit(
	Semaphore* Sem,
	uint32_t Value
	)
{
	int Status = sem_init(Sem, 0, Value);
	HardenedAssertEQ(Status, 0);
}


void
SemaphoreDestroy(
	Semaphore* Sem
	)
{
	int Status = sem_destroy(Sem);
	HardenedAssertEQ(Status, 0);
}


void
SemaphoreWait(
	Semaphore* Sem
	)
{
	int Status;
	while((Status = sem_wait(Sem)))
	{
		if(errno == EINTR)
		{
			continue;
		}

		HardenedAssertUnreachable();
	}
}


void
SemaphoreTimedWait(
	Semaphore* Sem,
	uint64_t Nanoseconds
	)
{
	struct timespec Time;
	Time.tv_sec = Nanoseconds / 1000000000;
	Time.tv_nsec = Nanoseconds % 1000000000;

	int Status;
	while((Status = sem_timedwait(Sem, &Time)))
	{
		if(errno == EINTR)
		{
			continue;
		}

		if(errno == ETIMEDOUT)
		{
			break;
		}

		HardenedAssertUnreachable();
	}
}


void
SemaphorePost(
	Semaphore* Sem
	)
{
	int Status = sem_post(Sem);
	AssertEQ(Status, 0);
}


typedef struct ThreadDataInternal
{
	ThreadData Data;

	Mutex Mtx;
}
ThreadDataInternal;


Static void*
ThreadFuncInternal(
	ThreadDataInternal* Internal
	)
{
	ThreadData Data = Internal->Data;

	MutexUnlock(&Internal->Mtx);

	Data.Func(Data.Arg);

	return NULL;
}


void
ThreadInit(
	ThreadT* Thread,
	ThreadData Data
	)
{
	ThreadT ID;

	ThreadDataInternal* Internal = AllocMalloc(sizeof(ThreadDataInternal));
	AssertNotNull(Internal);

	Internal->Data = Data;
	AssertNotNull(Data.Func);

	MutexInit(&Internal->Mtx);
	MutexLock(&Internal->Mtx);

	int Status = pthread_create(&ID, NULL,
		(void* (*)(void*)) ThreadFuncInternal, Internal);
	HardenedAssertEQ(Status, 0);

	if(Thread)
	{
		*Thread = ID;
	}

	MutexLock(&Internal->Mtx);
	MutexUnlock(&Internal->Mtx);

	MutexDestroy(&Internal->Mtx);
	AllocFree(sizeof(ThreadDataInternal), Internal);
}


void
ThreadDestroy(
	ThreadT* Thread
	)
{
	*Thread = -1;
}


void
ThreadCancelOn(
	void
	)
{
	int Status = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	AssertEQ(Status, 0);
}


void
ThreadCancelOff(
	void
	)
{
	int Status = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	AssertEQ(Status, 0);
}


void
ThreadAsyncOn(
	void
	)
{
	int Status = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	AssertEQ(Status, 0);
}


void
ThreadAsyncOff(
	void
	)
{
	int Status = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	AssertEQ(Status, 0);
}


void
ThreadDetach(
	ThreadT Thread
	)
{
	int Status = pthread_detach(Thread);
	HardenedAssertEQ(Status, 0);
}


void
ThreadJoin(
	ThreadT Thread
	)
{
	int Status = pthread_join(Thread, NULL);
	HardenedAssertEQ(Status, 0);
}


Static void
ThreadCancel(
	ThreadT Thread
	)
{
	int Status = pthread_cancel(Thread);
	HardenedAssertEQ(Status, 0);
}


Static ThreadT
ThreadSelf(
	void
	)
{
	return pthread_self();
}


Static bool
ThreadEqual(
	ThreadT A,
	ThreadT B
	)
{
	return pthread_equal(A, B);
}


void
ThreadCancelSync(
	ThreadT Thread
	)
{
	if(ThreadEqual(Thread, ThreadSelf()))
	{
		ThreadDetach(Thread);
		ThreadExit();
	}
	else
	{
		ThreadCancel(Thread);
		ThreadJoin(Thread);
	}
}


void
ThreadCancelAsync(
	ThreadT Thread
	)
{
	ThreadDetach(Thread);
	ThreadCancel(Thread);
}


void
ThreadExit(
	void
	)
{
	pthread_exit(NULL);
}


void
ThreadSleep(
	uint64_t Nanoseconds
	)
{
	struct timespec Time;
	Time.tv_sec = Nanoseconds / 1000000000;
	Time.tv_nsec = Nanoseconds % 1000000000;

	struct timespec Rem;

	int Status;
	while((Status = nanosleep(&Time, &Rem)))
	{
		if(errno == EINTR)
		{
			Time = Rem;
			continue;
		}

		HardenedAssertUnreachable();
	}
}


Static void
ThreadsResize(
	ThreadsT* Threads,
	uint32_t Count
	)
{
	uint32_t NewUsed = Threads->Used + Count;
	uint32_t NewSize;

	if((NewUsed < (Threads->Size >> 2)) || (NewUsed > Threads->Size))
	{
		NewSize = (NewUsed << 1) | 1;
	}
	else
	{
		return;
	}

	Threads->Threads = AllocRemalloc(sizeof(ThreadT) * Threads->Size,
		Threads->Threads, sizeof(ThreadT) * NewSize);
	AssertNotNull(Threads->Threads);

	Threads->Size = NewSize;
}


void
ThreadsInit(
	ThreadsT* Threads
	)
{
	Threads->Threads = NULL;
	Threads->Used = 0;
	Threads->Size = 0;
}


void
ThreadsDestroy(
	ThreadsT* Threads
	)
{
	AllocFree(sizeof(ThreadT) * Threads->Size, Threads->Threads);
}


void
ThreadsAdd(
	ThreadsT* Threads,
	ThreadData Data,
	uint32_t Count
	)
{
	AssertGT(Count, 0);

	ThreadsResize(Threads, Count);

	ThreadT* Thread = Threads->Threads + Threads->Used;
	ThreadT* ThreadEnd = Thread + Count;

	do
	{
		ThreadInit(Thread, Data);
	}
	while(++Thread != ThreadEnd);

	Threads->Used += Count;
}


void
ThreadsCancelSync(
	ThreadsT* Threads,
	uint32_t Count
	)
{
	AssertGT(Count, 0);
	AssertLE(Count, Threads->Used);

	ThreadT* ThreadStart = Threads->Threads + Threads->Used - Count;

	ThreadT* Thread = ThreadStart;
	ThreadT* ThreadEnd = Thread + Count;

	ThreadT Self = ThreadSelf();
	bool FoundOurself = false;

	do
	{
		if(ThreadEqual(*Thread, Self))
		{
			FoundOurself = true;
		}
		else
		{
			ThreadCancel(*Thread);
		}
	}
	while(++Thread != ThreadEnd);

	Thread = ThreadStart;
	do
	{
		if(!ThreadEqual(*Thread, Self))
		{
			ThreadJoin(*Thread);
		}
	}
	while(++Thread != ThreadEnd);

	ThreadsResize(Threads, -Count);

	if(FoundOurself)
	{
		ThreadDetach(Self);
		ThreadExit();
	}
}


void
ThreadsCancelAsync(
	ThreadsT* Threads,
	uint32_t Count
	)
{
	AssertGT(Count, 0);
	AssertLE(Count, Threads->Used);

	ThreadT* ThreadStart = Threads->Threads + Threads->Used - Count;

	ThreadT* Thread = ThreadStart;
	ThreadT* ThreadEnd = Thread + Count;

	ThreadT Self = ThreadSelf();
	bool FoundOurself = false;

	do
	{
		if(ThreadEqual(*Thread, Self))
		{
			FoundOurself = true;
		}
		else
		{
			ThreadDetach(*Thread);
			ThreadCancel(*Thread);
		}
	}
	while(++Thread != ThreadEnd);

	ThreadsResize(Threads, -Count);

	if(FoundOurself)
	{
		ThreadDetach(Self);
		ThreadCancel(Self);
	}
}


void
ThreadsShutdownSync(
	ThreadsT* Threads
	)
{
	ThreadsCancelSync(Threads, Threads->Used);
}


void
ThreadsShutdownAsync(
	ThreadsT* Threads
	)
{
	ThreadsCancelAsync(Threads, Threads->Used);
}


void
ThreadPoolFunc(
	void* Data
	)
{
	ThreadPoolT* Pool = Data;

	while(1)
	{
		ThreadPoolWork(Pool);
	}
}


void
ThreadPoolInit(
	ThreadPoolT* Pool
	)
{
	SemaphoreInit(&Pool->Sem, 0);
	MutexInit(&Pool->Mtx);

	Pool->Queue = NULL;
	Pool->Used = 0;
	Pool->Size = 0;
}


void
ThreadPoolDestroy(
	ThreadPoolT* Pool
	)
{
	AllocFree(sizeof(ThreadData) * Pool->Size, Pool->Queue);

	MutexDestroy(&Pool->Mtx);
	SemaphoreDestroy(&Pool->Sem);
}


void
ThreadPoolLock(
	ThreadPoolT* Pool
	)
{
	MutexLock(&Pool->Mtx);
}


void
ThreadPoolUnlock(
	ThreadPoolT* Pool
	)
{
	MutexUnlock(&Pool->Mtx);
}


Static void
ThreadPoolResize(
	ThreadPoolT* Pool,
	uint32_t Count
	)
{
	uint32_t NewUsed = Pool->Used + Count;
	uint32_t NewSize;

	if((NewUsed < (Pool->Size >> 2)) || (NewUsed > Pool->Size))
	{
		NewSize = (NewUsed << 1) | 1;
	}
	else
	{
		return;
	}

	Pool->Queue = AllocRemalloc(sizeof(ThreadData) * Pool->Size,
		Pool->Queue, sizeof(ThreadData) * NewSize);
	AssertNotNull(Pool->Queue);

	Pool->Size = NewSize;
}


Static void
ThreadPoolAddCommon(
	ThreadPoolT* Pool,
	ThreadData Data,
	bool Lock
	)
{
	if(Lock)
	{
		ThreadPoolLock(Pool);
	}

	ThreadPoolResize(Pool, 1);

	Pool->Queue[Pool->Used++] = Data;

	if(Lock)
	{
		ThreadPoolUnlock(Pool);
	}

	SemaphorePost(&Pool->Sem);
}


void
ThreadPoolAddU(
	ThreadPoolT* Pool,
	ThreadData Data
	)
{
	ThreadPoolAddCommon(Pool, Data, false);
}


void
ThreadPoolAdd(
	ThreadPoolT* Pool,
	ThreadData Data
	)
{
	ThreadPoolAddCommon(Pool, Data, true);
}


Static void
ThreadPoolTryWorkCommon(
	ThreadPoolT* Pool,
	bool Lock
	)
{
	if(Lock)
	{
		ThreadPoolLock(Pool);
	}

	if(!Pool->Used)
	{
		if(Lock)
		{
			ThreadPoolUnlock(Pool);
		}

		return;
	}

	ThreadData Data = *Pool->Queue;

	ThreadPoolResize(Pool, -1);

	if(Pool->Used)
	{
		(void) memmove(Pool->Queue, Pool->Queue + 1,
			sizeof(ThreadData) * Pool->Used);
	}

	if(Lock)
	{
		ThreadPoolUnlock(Pool);
	}

	Data.Func(Data.Arg);
}


void
ThreadPoolTryWorkU(
	ThreadPoolT* Pool
	)
{
	ThreadPoolTryWorkCommon(Pool, false);
}


void
ThreadPoolTryWork(
	ThreadPoolT* Pool
	)
{
	ThreadPoolTryWorkCommon(Pool, true);
}


void
ThreadPoolWorkU(
	ThreadPoolT* Pool
	)
{
	SemaphoreWait(&Pool->Sem);

	ThreadPoolTryWorkU(Pool);
}


void
ThreadPoolWork(
	ThreadPoolT* Pool
	)
{
	ThreadAsyncOff();
		SemaphoreWait(&Pool->Sem);

		ThreadCancelOff();
			ThreadPoolTryWork(Pool);
		ThreadCancelOn();
	ThreadAsyncOn();
}
