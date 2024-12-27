#pragma once

#include <DiepDesktop/shared/threads.h>


extern uint64_t
TimeSecondsToMilliseconds(
	uint64_t Seconds
	);


extern uint64_t
TimeSecondsToMicroseconds(
	uint64_t Seconds
	);


extern uint64_t
TimeSecondsToNanoseconds(
	uint64_t Seconds
	);


extern uint64_t
TimeMillisecondsToSeconds(
	uint64_t Milliseconds
	);


extern uint64_t
TimeMillisecondsToMicroseconds(
	uint64_t Milliseconds
	);


extern uint64_t
TimeMillisecondsToNanoseconds(
	uint64_t Milliseconds
	);


extern uint64_t
TimeMicrosecondsToSeconds(
	uint64_t Microseconds
	);


extern uint64_t
TimeMicrosecondsToMilliseconds(
	uint64_t Microseconds
	);


extern uint64_t
TimeMicrosecondsToNanoseconds(
	uint64_t Microseconds
	);


extern uint64_t
TimeNanosecondsToSeconds(
	uint64_t Nanoseconds
	);


extern uint64_t
TimeNanosecondsToMilliseconds(
	uint64_t Nanoseconds
	);


extern uint64_t
TimeNanosecondsToMicroseconds(
	uint64_t Nanoseconds
	);


extern uint64_t
TimeGet(
	void
	);


extern uint64_t
TimeGetWithSeconds(
	uint64_t Seconds
	);


extern uint64_t
TimeGetWithMilliseconds(
	uint64_t Milliseconds
	);


extern uint64_t
TimeGetWithMicroseconds(
	uint64_t Microseconds
	);


extern uint64_t
TimeGetWithNanoseconds(
	uint64_t Nanoseconds
	);


typedef struct TimeTimer
{
	uint32_t Index;
}
TimeTimer;


typedef void
(*TimeFunc)(
	void* Arg
	);

typedef struct TimeData
{
	TimeFunc Func;
	void* Arg;
}
TimeData;


typedef struct TimeTimeout
{
	TimeTimer* Timer;

	TimeData Data;

	uint64_t Time;
}
TimeTimeout;


typedef struct TimeInterval
{
	TimeTimer* Timer;

	TimeData Data;

	uint64_t BaseTime;
	uint64_t Interval;
	uint64_t Count;
}
TimeInterval;


typedef struct TimeTimers
{
	TimeTimeout* Timeouts;
	uint32_t TimeoutsUsed;
	uint32_t TimeoutsSize;

	TimeInterval* Intervals;
	uint32_t IntervalsUsed;
	uint32_t IntervalsSize;

	ThreadT Thread;
	Mutex Mtx;
	Semaphore WorkSem;
	Semaphore UpdatesSem;

	_Atomic uint64_t Latest;
}
TimeTimers;


extern void
TimeThreadFunc(
	void* Data
	);


extern void
TimeInit(
	TimeTimers* Timers
	);


extern void
TimeDestroy(
	TimeTimers* Timers
	);


extern void
TimeLock(
	TimeTimers* Timers
	);


extern void
TimeUnlock(
	TimeTimers* Timers
	);



extern void
TimeAddTimeoutU(
	TimeTimers* Timers,
	TimeTimeout Timeout
	);


extern void
TimeAddTimeout(
	TimeTimers* Timers,
	TimeTimeout Timeout
	);


extern bool
TimeCancelTimeoutU(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern bool
TimeCancelTimeout(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern TimeTimeout*
TimeOpenTimeoutU(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern TimeTimeout*
TimeOpenTimeout(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern void
TimeCloseTimeoutU(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern void
TimeCloseTimeout(
	TimeTimers* Timers,
	TimeTimer* Timer
	);



extern void
TimeAddIntervalU(
	TimeTimers* Timers,
	TimeInterval Interval
	);


extern void
TimeAddInterval(
	TimeTimers* Timers,
	TimeInterval Interval
	);


extern bool
TimeCancelIntervalU(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern bool
TimeCancelInterval(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern TimeInterval*
TimeOpenIntervalU(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern TimeInterval*
TimeOpenInterval(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern void
TimeCloseIntervalU(
	TimeTimers* Timers,
	TimeTimer* Timer
	);


extern void
TimeCloseInterval(
	TimeTimers* Timers,
	TimeTimer* Timer
	);
