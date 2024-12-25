#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/shared/alloc_ext.h>


typedef struct ThreadData
{
	ThreadType Func;
	void* Arg;
	HANDLE Lock;
}
ThreadData;


Static DWORD WINAPI
WinThread(
	LPVOID _Data
	)
{
	ThreadData* Data = _Data;
	ThreadType Func = Data->Func;
	void* Arg = Data->Arg;
	SetEvent(Data->Lock);

	Func(Arg);

	return 0;
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
	Data->Lock = CreateEvent(NULL, FALSE, FALSE, NULL);

	*Thread = CreateThread(NULL, 0, WinThread, Data, 0, NULL);
	AssertNotNull(*Thread);

	WaitForSingleObject(Data->Lock, INFINITE);
	CloseHandle(Data->Lock);
	AllocFree(sizeof(*Data), Data);
}


void
ThreadWait(
	ThreadID Thread
	)
{
	WaitForSingleObject(Thread, INFINITE);
	CloseHandle(Thread);
}


void
ThreadQuit(
	void
	)
{
	TerminateThread(GetCurrentThread(), 0);
}


void
ThreadDestroy(
	ThreadID Thread
	)
{
	TerminateThread(Thread, 0);
	ThreadWait(Thread);
}


void
MutexInit(
	Mutex* Mtx
	)
{
	InitializeCriticalSection(Mtx);
}


void
MutexDestroy(
	Mutex* Mtx
	)
{
	DeleteCriticalSection(Mtx);
}


void
MutexLock(
	Mutex* Mtx
	)
{
	EnterCriticalSection(Mtx);
}


void
MutexUnlock(
	Mutex* Mtx
	)
{
	LeaveCriticalSection(Mtx);
}


void
CondVarInit(
	CondVar* Var
	)
{
	InitializeConditionVariable(Var);
}


void
CondVarDestroy(
	CondVar* Var
	)
{
	(void) Var;
}


void
CondVarWait(
	CondVar* Var,
	Mutex* Mtx
	)
{
	SleepConditionVariableCS(Var, Mtx, INFINITE);
}


void
CondVarWake(
	CondVar* Var
	)
{
	WakeConditionVariable(Var);
}
