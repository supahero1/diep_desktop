#pragma once

#include <stdint.h>


typedef void
(*ListenerCallback)(
	void* Data,
	void* EventData
	);


typedef struct Listener
{
	ListenerCallback Callback;
	void* Data;
}
Listener;


typedef struct ListenerNode ListenerNode;

struct ListenerNode
{
	ListenerNode* Prev;
	ListenerNode* Next;
	Listener Listener;
};


typedef struct EventTarget
{
	ListenerNode* Head;
}
EventTarget;


extern void
EventListen(
	EventTarget* Target,
	ListenerCallback Callback,
	void* Data
	);


extern void
EventUnlisten(
	EventTarget* Target,
	ListenerCallback Callback,
	void* Data
	);


extern void
EventNotify(
	EventTarget* Target,
	void* EventData
	);


extern bool
EventHasListeners(
	EventTarget* Target
	);


extern void
EventFree(
	EventTarget* Target
	);
