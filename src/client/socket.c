#include "../include/socket.h"
#include "../include/debug.h"

#include <stdlib.h> // todo alloc_ext.h
#include <string.h>

#ifdef _WIN32
	#include <ws2tcpip.h>
#else
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
#endif


Static void
TcpSocketThreadFN(
	void* _Socket
	)
{
	TcpSocket* Socket = _Socket;

	struct sockaddr_in Addr = {0};
	Addr.sin_family = AF_INET;
	inet_pton(AF_INET, Socket->Host, &Addr.sin_addr);
	Addr.sin_port = htons(Socket->Port);

	SocketID ID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	AssertNEQ(ID, -1);

	int Error = setsockopt(ID, SOL_SOCKET, SO_RCVBUF, (const char*) &Socket->BufferSize, 4);
	AssertNEQ(Error, -1);

	Error = connect(ID, (struct sockaddr*) &Addr, sizeof(Addr));

	if(Error)
	{
		goto goto_close;
	}

	atomic_store_explicit(&Socket->ID, ID, memory_order_release);
	Socket->Open();

	while(1)
	{
		int Bytes = recv(ID, (void*)(Socket->Buffer + Socket->BufferUsed), Socket->BufferSize - Socket->BufferUsed, 0);

		if(Bytes <= 0)
		{
			goto goto_close;
		}

		Socket->BufferUsed += Bytes;

		while(1)
		{
			uint32_t Read = Socket->Read(Socket->Buffer, Socket->BufferUsed);

			if(Read == 0)
			{
				break;
			}

			Socket->BufferUsed -= Read;

			if(!Socket->BufferUsed)
			{
				break;
			}

			(void) memmove(Socket->Buffer, Socket->Buffer + Read, Socket->BufferUsed);
		}
	}

	goto_close:

#ifdef _WIN32
	closesocket(ID);
#else
	close(ID);
#endif
	Socket->Close();
	ThreadQuit();
}


void
SocketInit(
	void* _Socket
	)
{
	TcpSocket* Socket = _Socket;

	if(Socket->Secure)
	{
		AssertUnreachable();
	}

	Socket->BufferUsed = 0;
	Socket->Buffer = malloc(Socket->BufferSize);
	AssertNotNull(Socket->Buffer);

	ThreadInit(&Socket->Thread, TcpSocketThreadFN, Socket);
}


void
SocketFree(
	void* _Socket
	)
{
	TcpSocket* Socket = _Socket;

	if(Socket->Secure)
	{
		AssertUnreachable();
	}

	ThreadDestroy(Socket->Thread);

	free(Socket->Buffer);
}


void
SocketSendData(
	void* _Socket,
	const void* Data,
	uint32_t Length
	)
{
	TcpSocket* Socket = _Socket;

	if(Socket->Secure)
	{
		AssertUnreachable();
	}

	int ID = atomic_load_explicit(&Socket->ID, memory_order_acquire);

	(void) send(ID, Data, Length, 0);
}
