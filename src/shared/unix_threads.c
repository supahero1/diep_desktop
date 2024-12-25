#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/shared/alloc_ext.h>


typedef struct ThreadData
{
	ThreadType Func;
	void* Arg;
	Mutex Lock;
}
ThreadData;


Static void*
UnixThread(
	void* _Data
	)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	ThreadData* Data = _Data;
	ThreadType Func = Data->Func;
	void* Arg = Data->Arg;
	MutexUnlock(&Data->Lock);

	Func(Arg);

	return NULL;
}


void
ThreadInit(
	ThreadID* Thread,
	ThreadType Func,
	void* Arg
	)
{
	ThreadData* Data = AllocMalloc(sizeof(*Data));
	AssertNotNull(Data);

	Data->Func = Func;
	Data->Arg = Arg;
	MutexInit(&Data->Lock);
	MutexLock(&Data->Lock);

	int Status = pthread_create(Thread, NULL, UnixThread, Data);
	AssertEQ(Status, 0);

	MutexLock(&Data->Lock);
	MutexUnlock(&Data->Lock);
	MutexDestroy(&Data->Lock);
	AllocFree(sizeof(*Data), Data);
}


void
ThreadWait(
	ThreadID Thread
	)
{
	pthread_join(Thread, NULL);
}


void
ThreadQuit(
	void
	)
{
	pthread_detach(pthread_self());
	pthread_exit(NULL);
}


void
ThreadDestroy(
	ThreadID Thread
	)
{
	pthread_cancel(Thread);
	ThreadWait(Thread);
}


void
MutexInit(
	Mutex* Mtx
	)
{
	pthread_mutex_init(Mtx, NULL);
}


void
MutexDestroy(
	Mutex* Mtx
	)
{
	pthread_mutex_destroy(Mtx);
}


void
MutexLock(
	Mutex* Mtx
	)
{
	pthread_mutex_lock(Mtx);
}


void
MutexUnlock(
	Mutex* Mtx
	)
{
	pthread_mutex_unlock(Mtx);
}


void
CondVarInit(
	CondVar* Var
	)
{
	pthread_cond_init(Var, NULL);
}


void
CondVarDestroy(
	CondVar* Var
	)
{
	pthread_cond_destroy(Var);
}


void
CondVarWait(
	CondVar* Var,
	Mutex* Mtx
	)
{
	pthread_cond_wait(Var, Mtx);
}


void
CondVarWake(
	CondVar* Var
	)
{
	pthread_cond_signal(Var);
}
