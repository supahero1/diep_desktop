#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/time.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <time.h>
#include <stdatomic.h>


uint64_t
TimeSecondsToMilliseconds(
	uint64_t Seconds
	)
{
	return Seconds * 1000;
}


uint64_t
TimeSecondsToMicroseconds(
	uint64_t Seconds
	)
{
	return Seconds * 1000000;
}


uint64_t
TimeSecondsToNanoseconds(
	uint64_t Seconds
	)
{
	return Seconds * 1000000000;
}


uint64_t
TimeMillisecondsToSeconds(
	uint64_t Milliseconds
	)
{
	return Milliseconds / 1000;
}


uint64_t
TimeMillisecondsToMicroseconds(
	uint64_t Milliseconds
	)
{
	return Milliseconds * 1000;
}


uint64_t
TimeMillisecondsToNanoseconds(
	uint64_t Milliseconds
	)
{
	return Milliseconds * 1000000;
}


uint64_t
TimeMicrosecondsToSeconds(
	uint64_t Microseconds
	)
{
	return Microseconds / 1000000;
}


uint64_t
TimeMicrosecondsToMilliseconds(
	uint64_t Microseconds
	)
{
	return Microseconds / 1000;
}


uint64_t
TimeMicrosecondsToNanoseconds(
	uint64_t Microseconds
	)
{
	return Microseconds * 1000;
}


uint64_t
TimeNanosecondsToSeconds(
	uint64_t Nanoseconds
	)
{
	return Nanoseconds / 1000000000;
}


uint64_t
TimeNanosecondsToMilliseconds(
	uint64_t Nanoseconds
	)
{
	return Nanoseconds / 1000000;
}


uint64_t
TimeNanosecondsToMicroseconds(
	uint64_t Nanoseconds
	)
{
	return Nanoseconds / 1000;
}


uint64_t
TimeGet(
	void
	)
{
	struct timespec Time;
	int Status = clock_gettime(CLOCK_REALTIME, &Time);
	HardenedAssertEQ(Status, 0);

	return Time.tv_sec * 1000000000 + Time.tv_nsec;
}


uint64_t
TimeGetWithSeconds(
	uint64_t Seconds
	)
{
	return TimeGet() + TimeSecondsToNanoseconds(Seconds);
}


uint64_t
TimeGetWithMilliseconds(
	uint64_t Milliseconds
	)
{
	return TimeGet() + TimeMillisecondsToNanoseconds(Milliseconds);
}


uint64_t
TimeGetWithMicroseconds(
	uint64_t Microseconds
	)
{
	return TimeGet() + TimeMicrosecondsToNanoseconds(Microseconds);
}


uint64_t
TimeGetWithNanoseconds(
	uint64_t Nanoseconds
	)
{
	return TimeGet() + Nanoseconds;
}


Static uint64_t
TimeGetLatest(
	TimeTimers* Timers
	)
{
	return atomic_load_explicit(&Timers->Latest, memory_order_acquire);
}


#define TimeOfTimeout(Index)				\
	Timers->Timeouts[Index].Time

#define TimeOfInterval(Index)				\
	Timers->Intervals[Index].BaseTime		\
		+ Timers->Intervals[Index].Interval	\
		* Timers->Intervals[Index].Count


Static void
TimeSetLatest(
	TimeTimers* Timers
	)
{
	uint64_t Old = TimeGetLatest(Timers);
	uint64_t Latest = UINT64_MAX;

	if(Timers->TimeoutsUsed > 1)
	{
		Latest = MIN(Latest, TimeOfTimeout(1));
	}

	if(Timers->IntervalsUsed > 1)
	{
		Latest = MIN(Latest, TimeOfInterval(1));
	}

	if(Latest == UINT64_MAX)
	{
		Latest = 0;
	}

	atomic_store_explicit(&Timers->Latest, Latest, memory_order_release);

	if(Old != Latest)
	{
		SemaphorePost(&Timers->UpdatesSem);
	}
}


#define TIME_TIMER_DEF(Name, Names)																\
																								\
Static void																						\
TimeSwap##Names (																				\
	TimeTimers* Timers,																			\
	uint32_t IndexA,																			\
	uint32_t IndexB																				\
	)																							\
{																								\
	Timers-> Names [IndexA] = Timers-> Names [IndexB];											\
																								\
	if(Timers-> Names [IndexA].Timer != NULL)													\
	{																							\
		Timers-> Names [IndexA].Timer-> Index = IndexA;											\
	}																							\
}																								\
																								\
																								\
Static void																						\
Time##Names##Down (																				\
	TimeTimers* Timers,																			\
	uint32_t Timer																				\
	)																							\
{																								\
	uint32_t Save = Timer;																		\
																								\
	Timers-> Names [0] = Timers-> Names [Timer];												\
																								\
	while(1)																					\
	{																							\
		uint32_t LeftChild = Timer << 1;														\
		uint32_t RightChild = LeftChild + 1;													\
		uint32_t Smallest = 0;																	\
																								\
		if(LeftChild < Timers-> Names##Used &&													\
			TimeOf##Name (LeftChild) < TimeOf##Name (Smallest))									\
		{																						\
			Smallest = LeftChild;																\
		}																						\
																								\
		if(RightChild < Timers-> Names##Used &&													\
			TimeOf##Name (RightChild) < TimeOf##Name (Smallest))								\
		{																						\
			Smallest = RightChild;																\
		}																						\
																								\
		if(Smallest == 0)																		\
		{																						\
			break;																				\
		}																						\
																								\
		TimeSwap##Names (Timers, Timer, Smallest);												\
		Timer = Smallest;																		\
	}																							\
																								\
	if(Save != Timer)																			\
	{																							\
		TimeSwap##Names (Timers, Timer, 0);														\
	}																							\
}																								\
																								\
																								\
Static bool																						\
Time##Names##Up (																				\
	TimeTimers* Timers,																			\
	uint32_t Timer																				\
	)																							\
{																								\
	uint32_t Save = Timer;																		\
	uint32_t Parent = Timer >> 1;																\
																								\
	Timers-> Names [0] = Timers-> Names [Timer];												\
																								\
	while(																						\
		Parent > 0 &&																			\
		TimeOf##Name (Parent) > TimeOf##Name (0)												\
		)																						\
	{																							\
		TimeSwap##Names (Timers, Timer, Parent);												\
		Timer = Parent;																			\
		Parent >>= 1;																			\
	}																							\
																								\
	if(Save != Timer)																			\
	{																							\
		TimeSwap##Names (Timers, Timer, 0);														\
		return true;																			\
	}																							\
																								\
	return false;																				\
}																								\
																								\
																								\
Static void																						\
TimeDestroy##Names (																			\
	TimeTimers* Timers																			\
	)																							\
{																								\
	AllocFree(sizeof( Time##Name ) * Timers-> Names##Size , Timers-> Names);					\
}																								\
																								\
																								\
Static void																						\
TimeResize##Names (																				\
	TimeTimers* Timers,																			\
	uint32_t Count																				\
	)																							\
{																								\
	uint32_t NewUsed = Timers-> Names##Used + Count;											\
	uint32_t NewSize;																			\
																								\
	if((NewUsed < (Timers-> Names##Size >> 2)) || (NewUsed > Timers-> Names##Size ))			\
	{																							\
		NewSize = (NewUsed << 1) | 1;															\
	}																							\
	else																						\
	{																							\
		return;																					\
	}																							\
																								\
	Timers-> Names = AllocRemalloc(sizeof( Time##Name ) * Timers-> Names##Size ,				\
		Timers-> Names, sizeof( Time##Name ) * NewSize);										\
	AssertNotNull(Timers-> Names);																\
																								\
	Timers-> Names##Size = NewSize;																\
}																								\
																								\
																								\
Static void																						\
TimeAdd##Name##Common (																			\
	TimeTimers* Timers,																			\
	Time##Name Name,																			\
	bool Lock																					\
	)																							\
{																								\
	if(Lock)																					\
	{																							\
		TimeLock(Timers);																		\
	}																							\
																								\
	TimeResize##Names (Timers, 1);																\
																								\
	if(Name.Timer != NULL)																		\
	{																							\
		Name.Timer->Index = Timers-> Names##Used ;												\
	}																							\
																								\
	Timers-> Names [Timers-> Names##Used ++] = Name;											\
																								\
	(void) Time##Names##Up (Timers, Timers-> Names##Used - 1);									\
																								\
	TimeSetLatest(Timers);																		\
																								\
	if(Lock)																					\
	{																							\
		TimeUnlock(Timers);																		\
	}																							\
																								\
	SemaphorePost(&Timers->WorkSem);															\
}																								\
																								\
																								\
void																							\
TimeAdd##Name##U (																				\
	TimeTimers* Timers,																			\
	Time##Name Name																				\
	)																							\
{																								\
	TimeAdd##Name##Common (Timers, Name, false);												\
}																								\
																								\
																								\
void																							\
TimeAdd##Name (																					\
	TimeTimers* Timers,																			\
	Time##Name Name																				\
	)																							\
{																								\
	TimeAdd##Name##Common (Timers, Name, true);													\
}																								\
																								\
																								\
bool																							\
TimeCancel##Name##U (																			\
	TimeTimers* Timers,																			\
	TimeTimer* Timer																			\
	)																							\
{																								\
	if(Timer->Index == 0)																		\
	{																							\
		return false;																			\
	}																							\
																								\
	Timers-> Names [Timer->Index] = Timers-> Names [-- Timers-> Names##Used ];					\
																								\
	if(! Time##Names##Up (Timers, Timer->Index))												\
	{																							\
		Time##Names##Down (Timers, Timer->Index);												\
	}																							\
																								\
	TimeSetLatest(Timers);																		\
																								\
	Timer->Index = 0;																			\
																								\
	return true;																				\
}																								\
																								\
																								\
bool																							\
TimeCancel##Name (																				\
	TimeTimers* Timers,																			\
	TimeTimer* Timer																			\
	)																							\
{																								\
	TimeLock(Timers);																			\
	bool Result = TimeCancel##Name##U (Timers, Timer);											\
	TimeUnlock(Timers);																			\
	return Result;																				\
}																								\
																								\
																								\
Time##Name *																					\
TimeOpen##Name##U (																				\
	TimeTimers* Timers,																			\
	TimeTimer* Timer																			\
	)																							\
{																								\
	if(Timer->Index == 0)																		\
	{																							\
		return NULL;																			\
	}																							\
																								\
	return &Timers-> Names [Timer->Index];														\
}																								\
																								\
																								\
Time##Name *																					\
TimeOpen##Name (																				\
	TimeTimers* Timers,																			\
	TimeTimer* Timer																			\
	)																							\
{																								\
	TimeLock(Timers);																			\
																								\
	Time##Name * Result = TimeOpen##Name##U (Timers, Timer);									\
																								\
	if(!Result)																					\
	{																							\
		TimeUnlock(Timers);																		\
	}																							\
																								\
	return Result;																				\
}																								\
																								\
																								\
void																							\
TimeClose##Name##U (																			\
	TimeTimers* Timers,																			\
	TimeTimer* Timer																			\
	)																							\
{																								\
	if(Timer->Index == 0)																		\
	{																							\
		return;																					\
	}																							\
																								\
	if(! Time##Names##Up (Timers, Timer->Index))												\
	{																							\
		Time##Names##Down (Timers, Timer->Index);												\
	}																							\
																								\
	TimeSetLatest(Timers);																		\
}																								\
																								\
																								\
void																							\
TimeClose##Name (																				\
	TimeTimers* Timers,																			\
	TimeTimer* Timer																			\
	)																							\
{																								\
	TimeClose##Name##U (Timers, Timer);															\
	TimeUnlock(Timers);																			\
}


TIME_TIMER_DEF(Timeout, Timeouts)
TIME_TIMER_DEF(Interval, Intervals)

#undef TIME_TIMER_DEF


void
TimeThreadFunc(
	void* Data
	)
{
	TimeTimers* Timers = Data;

	while(1)
	{
		goto_start:

		SemaphoreWait(&Timers->WorkSem);

		uint64_t Time;

		while(1)
		{
			Time = TimeGetLatest(Timers);

			if(Time == 0)
			{
				goto goto_start;
			}

			if(TimeGet() >= Time)
			{
				break;
			}

			SemaphoreTimedWait(&Timers->UpdatesSem, Time);
		}

		TimeData Data;

		TimeLock(Timers);

		Time = TimeGetLatest(Timers);

		if(Time == 0 || TimeGet() < Time)
		{
			TimeUnlock(Timers);
			goto goto_start;
		}

		if(Time & 1)
		{
			TimeInterval* Interval = &Timers->Intervals[1];
			Data = Interval->Data;

			++Interval->Count;
			TimeIntervalsDown(Timers, 1);
			SemaphorePost(&Timers->WorkSem);
		}
		else
		{
			TimeTimeout* Timeout = &Timers->Timeouts[1];
			Data = Timeout->Data;

			if(Timeout->Timer != NULL)
			{
				Timeout->Timer->Index = 0;
			}

			if(--Timers->TimeoutsUsed > 1)
			{
				TimeSwapTimeouts(Timers, 1, Timers->TimeoutsUsed);
				TimeTimeoutsDown(Timers, 1);
			}
		}

		TimeSetLatest(Timers);

		TimeUnlock(Timers);

		if(Data.Func != NULL)
		{
			ThreadCancelOff();
			Data.Func(Data.Arg);
			ThreadCancelOn();
		}
	}
}


void
TimeInit(
	TimeTimers* Timers
	)
{
	Timers->Timeouts = NULL;
	Timers->TimeoutsUsed = 1;
	Timers->TimeoutsSize = 0;

	Timers->Intervals = NULL;
	Timers->IntervalsUsed = 1;
	Timers->IntervalsSize = 0;

	atomic_init(&Timers->Latest, 0);

	MutexInit(&Timers->Mtx);
	SemaphoreInit(&Timers->WorkSem, 0);
	SemaphoreInit(&Timers->UpdatesSem, 0);

	ThreadData Data =
	(ThreadData)
	{
		.Func = TimeThreadFunc,
		.Data = Timers
	};

	ThreadInit(&Timers->Thread, Data);
}


void
TimeDestroy(
	TimeTimers* Timers
	)
{
	ThreadCancelSync(Timers->Thread);
	ThreadDestroy(&Timers->Thread);

	SemaphoreDestroy(&Timers->UpdatesSem);
	SemaphoreDestroy(&Timers->WorkSem);
	MutexDestroy(&Timers->Mtx);

	TimeDestroyIntervals(Timers);
	TimeDestroyTimeouts(Timers);
}


void
TimeLock(
	TimeTimers* Timers
	)
{
	MutexLock(&Timers->Mtx);
}


void
TimeUnlock(
	TimeTimers* Timers
	)
{
	MutexUnlock(&Timers->Mtx);
}
