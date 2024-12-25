#pragma once

#include <stdint.h>
#include <stdatomic.h>

#ifdef _WIN32
	#include <winsock2.h>

	typedef SOCKET SocketID;
#else
	#include <sys/socket.h>

	typedef int SocketID;
#endif

#include <DiepDesktop/threads.h>


typedef void
(*SocketCallback)(
	void
	);


typedef uint32_t
(*SocketReadCallback)(
	uint8_t* Data,
	uint32_t Length
	);


typedef struct TcpSocket
{
	_Atomic SocketID ID;
	const char* Host;
	uint8_t* Buffer;
	SocketCallback Open;
	SocketReadCallback Read;
	SocketCallback Close;
	ThreadID Thread;
	uint16_t Port;
	uint16_t Secure;
	uint32_t BufferUsed;
	uint32_t BufferSize;
}
TcpSocket;


extern void
SocketInit(
	void* Socket
	);


extern void
SocketFree(
	void* Socket
	);


extern void
SocketSendData(
	void* Socket,
	const void* Data,
	uint32_t Length
	);


extern void
SocketSendInputPacket(
	TcpSocket* Socket,
	uint32_t MouseX,
	uint32_t MouseY,
	uint32_t Keys
	);
