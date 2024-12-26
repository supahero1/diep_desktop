#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/event.h>
#include <DiepDesktop/shared/alloc_ext.h>


void
EventListen(
	EventTarget* Target,
	ListenerCallback Callback,
	void* Data
	)
{
	AssertNotNull(Target);
	AssertNotNull(Callback);

	ListenerNode* Node = AllocMalloc(sizeof(ListenerNode));
	AssertNotNull(Node);

	Node->Prev = NULL;
	Node->Next = Target->Head;
	Node->Listener.Callback = Callback;
	Node->Listener.Data = Data;

	if(Target->Head)
	{
		Target->Head->Prev = Node;
	}

	Target->Head = Node;
}


void
EventUnlisten(
	EventTarget* Target,
	ListenerCallback Callback,
	void* Data
	)
{
	AssertNotNull(Target);
	AssertNotNull(Callback);

	ListenerNode* Node = Target->Head;
	while(Node)
	{
		if(Node->Listener.Callback == Callback && Node->Listener.Data == Data)
		{
			if(Node->Prev)
			{
				Node->Prev->Next = Node->Next;
			}
			else
			{
				Target->Head = Node->Next;
			}

			if(Node->Next)
			{
				Node->Next->Prev = Node->Prev;
			}

			AllocFree(sizeof(ListenerNode), Node);

			return;
		}

		Node = Node->Next;
	}

	AssertUnreachable();
}


void
EventNotify(
	EventTarget* Target,
	void* EventData
	)
{
	AssertNotNull(Target);

	ListenerNode* Node = Target->Head;
	while(Node)
	{
		Node->Listener.Callback(Node->Listener.Data, EventData);

		Node = Node->Next;
	}
}


bool
EventHasListeners(
	EventTarget* Target
	)
{
	AssertNotNull(Target);

	return !!Target->Head;
}


void
EventFree(
	EventTarget* Target
	)
{
	AssertNotNull(Target);

	ListenerNode* Node = Target->Head;
	while(Node != NULL)
	{
		ListenerNode* Next = Node->Next;

		AllocFree(sizeof(ListenerNode), Node);

		Node = Next;
	}
}
