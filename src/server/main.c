#include "../include/quadtree.h"
#include "../include/shared.h"
#include "../include/debug.h"
#include "../include/rand.h"
#include "../include/sort.h"
#include "../include/io.h"

#include <math.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>


typedef struct GameClient
{
	int FD;
	uint32_t Next;

	uint64_t ConnectionIdle;
	uint64_t ActionIdle;

	float CameraX;
	float CameraY;
	float CameraW;
	float CameraH;
	float FoV;

	float Vertical;
	float Horizontal;

	float MovementVX;
	float MovementVY;

	uint32_t BodyIndex;

	uint8_t Buffer[GAME_CONST_CLIENT_PACKET_SIZE];
	uint32_t BufferUsed;

	uint8_t Valid:1;

	uint8_t* EntityBits;
	uint16_t* EntitiesInView;
	uint16_t EntitiesInViewCount;
}
GameClient;


Static GameClient Clients[GAME_CONST_MAX_PLAYERS];
Static uint32_t ClientsUsed = 0;
Static uint32_t FreeClient = -1;

Static uint8_t* EntityBits;
Static uint16_t* EntitiesInView;
Static uint16_t EntitiesInViewCount;

Static GameClient* Client = NULL;
Static uint64_t CurrentTick = 0;
Static uint64_t LastTickAt;
Static uint64_t CurrentTickAt;

Static QUADTREE Quadtree = {0};

typedef struct GameEntity
{
	EntityType Type;
	uint32_t Subtype;

	float VX;
	float VY;

	float ColVX;
	float ColVY;

	float Angle;
	float AngleDir;

	uint32_t HP;
	uint32_t MaxHP;

	uint64_t DamagedAt;

	uint32_t ConstrainToArena:1;
	uint32_t ResetPX:1;
	uint32_t ResetNX:1;
	uint32_t ResetPY:1;
	uint32_t ResetNY:1;

	uint32_t UpdateHP:1;
	uint32_t TookDamage:1;
}
GameEntity;

Static GameEntity* Entities = NULL;


Static GameEntity*
GetEntity(
	const QUADTREE_ENTITY* Entity
	)
{
	uint32_t Index = QuadtreeInsert(&Quadtree, Entity);
	AssertEQ(Quadtree.EntitiesSize, GAME_CONST_MAX_ENTITIES);

	GameEntity* Ret = Entities + Index;
	*Ret = (GameEntity){ 0 };

	return Ret;
}


Static void
RetEntity(
	const GameEntity* Entity
	)
{
	QuadtreeRemove(&Quadtree, Entity - Entities);
}


Static GameClient*
GetClient(
	void
	)
{
	GameClient* Ret = Clients;

	if(FreeClient != -1)
	{
		Ret += FreeClient;
		FreeClient = Ret->Next;
	}
	else
	{
		Ret += ClientsUsed++;
	}

	*Ret = (GameClient){ 0 };
	Ret->Valid = 1;

	return Ret;
}


Static void
RetClient(
	void
	)
{
	Client->Next = FreeClient;
	Client->Valid = 0;
	FreeClient = Client - Clients;
}


Static float
LerpF(
	float Old,
	float New,
	float By
	)
{
	return Old + (New - Old) * By;
}


Static void
GetSpawnCoords(
	QUADTREE_ENTITY* QTEntity
	)
{
	QTEntity->X = RandF() * (GAME_CONST_HALF_ARENA_SIZE * 2 - QTEntity->W) - GAME_CONST_HALF_ARENA_SIZE + QTEntity->W;
	QTEntity->Y = RandF() * (GAME_CONST_HALF_ARENA_SIZE * 2 - QTEntity->H) - GAME_CONST_HALF_ARENA_SIZE + QTEntity->H;
}


Static void
SpawnShape(
	Shape Subtype
	)
{
	QUADTREE_ENTITY QTEntity =
	{
		.W = ShapeHitbox[Subtype],
		.H = ShapeHitbox[Subtype]
	};

	GetSpawnCoords(&QTEntity);

	GameEntity* Entity = GetEntity(&QTEntity);

	Entity->Type = ENTITY_TYPE_SHAPE;
	Entity->Subtype = Subtype;
	Entity->Angle = RandAngle();
	Entity->AngleDir = RandBit() ? -1 : 1;
	Entity->MaxHP = ShapeMaxHP[Subtype];
	Entity->HP = MIN(Entity->MaxHP, RandF() * Entity->MaxHP * 6);
	Entity->ConstrainToArena = 1;
}


Static int
QuadtreeUpdateFN(
	QUADTREE* Quadtree,
	uint32_t EntityIdx
	)
{
	QUADTREE_ENTITY* QTEntity = Quadtree->Entities + EntityIdx;
	GameEntity* Entity = Entities + EntityIdx;

	switch(Entity->Type)
	{

	case ENTITY_TYPE_TANK:
	{
		break;
	}

	case ENTITY_TYPE_SHAPE:
	{
		Entity->VX = cosf(Entity->Angle) * 0.3f;
		Entity->VY = sinf(Entity->Angle) * 0.3f;
		Entity->Angle += 0.0005f * Entity->AngleDir;

		break;
	}

	default: __builtin_unreachable();

	}

	QTEntity->X += Entity->ColVX;
	QTEntity->Y += Entity->ColVY;

	Entity->ColVX = LerpF(Entity->ColVX, 0, 0.095f);
	Entity->ColVY = LerpF(Entity->ColVY, 0, 0.095f);

	QTEntity->X += Entity->VX;
	QTEntity->Y += Entity->VY;

	if(Entity->ConstrainToArena)
	{
		float Limit = GAME_CONST_HALF_ARENA_SIZE + GAME_CONST_BORDER_PADDING;

		if(QTEntity->X - QTEntity->W < -Limit)
		{
			QTEntity->X = -Limit + QTEntity->W;
			Entity->ResetNX = 1;
		}
		else if(QTEntity->X + QTEntity->W > Limit)
		{
			QTEntity->X = Limit - QTEntity->W;
			Entity->ResetPX = 1;
		}
		else
		{
			Entity->ResetPX = 0;
			Entity->ResetNX = 0;
		}

		if(QTEntity->Y - QTEntity->H < -Limit)
		{
			QTEntity->Y = -Limit + QTEntity->H;
			Entity->ResetNY = 1;
		}
		else if(QTEntity->Y + QTEntity->H > Limit)
		{
			QTEntity->Y = Limit - QTEntity->H;
			Entity->ResetPY = 1;
		}
		else
		{
			Entity->ResetPY = 0;
			Entity->ResetNY = 0;
		}
	}

	Entity->UpdateHP = 0;
	Entity->TookDamage = 0;

	if(Entity->HP != Entity->MaxHP && CurrentTick - Entity->DamagedAt >= 200)
	{
		Entity->HP = MIN(Entity->MaxHP, Entity->HP + 1);
		Entity->UpdateHP = 1;
	}

	return 1;
}


Static int
QuadtreeIsCollidingFN(
	const QUADTREE* Quadtree,
	uint32_t EntityIdxA,
	uint32_t EntityIdxB
	)
{
	QUADTREE_ENTITY* QTEntityA = Quadtree->Entities + EntityIdxA;
	QUADTREE_ENTITY* QTEntityB = Quadtree->Entities + EntityIdxB;

	float DiffX = QTEntityA->X - QTEntityB->X;
	float DiffY = QTEntityA->Y - QTEntityB->Y;
	float SumR = QTEntityA->R + QTEntityB->R;

	return DiffX * DiffX + DiffY * DiffY < SumR * SumR;
}


Static void
QuadtreeCollideFN(
	QUADTREE* Quadtree,
	uint32_t EntityIdxA,
	uint32_t EntityIdxB
	)
{
	QUADTREE_ENTITY* QTEntityA = Quadtree->Entities + EntityIdxA;
	GameEntity* EntityA = Entities + EntityIdxA;

	QUADTREE_ENTITY* QTEntityB = Quadtree->Entities + EntityIdxB;
	GameEntity* EntityB = Entities + EntityIdxB;

	float DiffX = QTEntityA->X - QTEntityB->X;
	float DiffY = QTEntityA->Y - QTEntityB->Y;

	float Dist = sqrtf(DiffX * DiffX + DiffY * DiffY);

	if(!Dist)
	{
		Dist = 1.0f;
	}

	DiffX /= Dist;
	DiffY /= Dist;

	EntityA->ColVX += DiffX * 5.0f;
	EntityA->ColVY += DiffY * 5.0f;

	EntityB->ColVX -= DiffX * 5.0f;
	EntityB->ColVY -= DiffY * 5.0f;

	if(EntityA->HP)
	{
		--EntityA->HP;
	}
	EntityA->UpdateHP = 1;
	EntityA->TookDamage = 1;
	EntityA->DamagedAt = CurrentTick;

	if(EntityB->HP)
	{
		--EntityB->HP;
	}
	EntityB->UpdateHP = 1;
	EntityB->TookDamage = 1;
	EntityB->DamagedAt = CurrentTick;
}


Static void
QuadtreeViewQueryFN(
	QUADTREE* Quadtree,
	uint32_t EntityIdx
	)
{
	EntityBits[EntityIdx >> 3] |= 1 << (EntityIdx & 7);
	EntitiesInView[EntitiesInViewCount++] = EntityIdx;
}


Static void
ClientChangeFoV(
	float FoV
	)
{
	Client->FoV = FoV;
	Client->CameraW = 960.0f / Client->FoV + 200.0f;
	Client->CameraH = 540.0f / Client->FoV + 200.0f;
}


Static void
ClientCreate(
	void
	)
{
	Client->ConnectionIdle = CurrentTick;
	Client->ActionIdle = CurrentTick;

	ClientChangeFoV(0.5f);

	GameEntity* Body = GetEntity(&(
		(QUADTREE_ENTITY)
		{
			.X = 0,
			.Y = 0,
			.W = 70,
			.H = 70
		}
	));
	Body->ConstrainToArena = 1;
	Body->MaxHP = 1000;
	Body->HP = Body->MaxHP;

	Client->BodyIndex = Body - Entities;

	Client->EntityBits = calloc(TO_BYTES(GAME_CONST_MAX_ENTITIES), sizeof(uint8_t));
	AssertNotNull(Client->EntityBits);
}


Static void
ClientClose(
	void
	)
{
	shutdown(Client->FD, SHUT_RDWR);
}


Static void
ClientSend(
	const BitBuffer* Buffer
	)
{
	ssize_t Bytes = send(Client->FD, Buffer->Buffer, Buffer->Length, MSG_NOSIGNAL);

	if(Bytes != Buffer->Length)
	{
		ClientClose();
	}
}


Static uint32_t
ClientRead(
	void
	)
{
	BitBuffer Buffer = {0};
	Buffer.Buffer = Client->Buffer;
	Buffer.At = Buffer.Buffer;
	Buffer.Length = Client->BufferUsed;

	if(Buffer.Length < TO_BYTES(CLIENT_OPCODE__BITS))
	{
		return 0;
	}

	uintptr_t OpCode = BitBufferGetBits(&Buffer, CLIENT_OPCODE__BITS);

	switch(OpCode)
	{

	case CLIENT_OPCODE_INPUT:
	{
		if(Buffer.Length < TO_BYTES(
			CLIENT_OPCODE__BITS +
			FIELD_SIZE_MOUSE_X +
			FIELD_SIZE_MOUSE_Y +
			KEY_BUTTON__BITS))
		{
			break;
		}

		uintptr_t MouseX = BitBufferGetBits(&Buffer, FIELD_SIZE_MOUSE_X);
		if(MouseX > 1920)
		{
			ClientClose();
			break;
		}

		uintptr_t MouseY = BitBufferGetBits(&Buffer, FIELD_SIZE_MOUSE_Y);
		if(MouseY > 1080)
		{
			ClientClose();
			break;
		}

		uintptr_t Keys = BitBufferGetBits(&Buffer, KEY_BUTTON__BITS);

		float Vertical = 0;
		float Horizontal = 0;

		if(Keys & SHIFT(KEY_BUTTON_W))
		{
			--Vertical;
		}

		if(Keys & SHIFT(KEY_BUTTON_A))
		{
			--Horizontal;
		}

		if(Keys & SHIFT(KEY_BUTTON_S))
		{
			++Vertical;
		}

		if(Keys & SHIFT(KEY_BUTTON_D))
		{
			++Horizontal;
		}

		if(Vertical || Horizontal)
		{
			float Dist = 1 / sqrtf(Vertical * Vertical + Horizontal * Horizontal);

			Vertical *= Dist;
			Horizontal *= Dist;
		}

		Client->Vertical = Vertical * GAME_CONST_MAX_MOVEMENT_SPEED;
		Client->Horizontal = Horizontal * GAME_CONST_MAX_MOVEMENT_SPEED;

		return BitBufferGetConsumed(&Buffer);
	}

	default:
	{
		ClientClose();

		break;
	}

	}

	return 0;
}


Static void
ClientDestroy(
	void
	)
{
	RetEntity(Entities + Client->BodyIndex);
	free(Client->EntityBits);
}


Static void
GameUpdate(
	void
	)
{
	Client = Clients;
	GameClient* ClientEnd = Clients + ClientsUsed;

	for(; Client != ClientEnd; ++Client)
	{
		if(!Client->Valid)
		{
			continue;
		}

		QUADTREE_ENTITY* QTEntity = Quadtree.Entities + Client->BodyIndex;
		GameEntity* Entity = Entities + Client->BodyIndex;

		Client->MovementVX = LerpF(Client->MovementVX, Client->Horizontal, 0.095f);
		Client->MovementVY = LerpF(Client->MovementVY, Client->Vertical, 0.095f);

		if((Entity->ResetPX && Client->MovementVX > 0) || (Entity->ResetNX && Client->MovementVX < 0))
		{
			Client->MovementVX = 0;
		}

		if((Entity->ResetPY && Client->MovementVY > 0) || (Entity->ResetNY && Client->MovementVY < 0))
		{
			Client->MovementVY = 0;
		}

		QTEntity->X += Client->MovementVX;
		QTEntity->Y += Client->MovementVY;
	}

	QuadtreeUpdate(&Quadtree);
	QuadtreeCollide(&Quadtree);

	Client = Clients;
	Quadtree.Query = QuadtreeViewQueryFN;

	for(; Client != ClientEnd; ++Client)
	{
		if(!Client->Valid)
		{
			continue;
		}

		QUADTREE_ENTITY* QTEntity = Quadtree.Entities + Client->BodyIndex;

		Client->CameraX = QTEntity->X;
		Client->CameraY = QTEntity->Y;

		BitBuffer Buffer = {0};
		Buffer.Length = GAME_CONST_SERVER_PACKET_SIZE;
		Buffer.Buffer = calloc(Buffer.Length, sizeof(uint8_t));
		AssertNotNull(Buffer.Buffer);
		Buffer.At = Buffer.Buffer;

		BitBufferSetBits(&Buffer, SERVER_OPCODE_UPDATE, SERVER_OPCODE__BITS);

		BitBufferContext PacketLength = BitBufferSave(&Buffer);
		BitBufferSkipBits(&Buffer, GAME_CONST_SERVER_PACKET_SIZE__BITS);

		BitBufferSetBits(&Buffer, (CurrentTickAt - LastTickAt) / 10000, FIELD_SIZE_TICK_DURATION);

		BitBufferSetFixedPoint(&Buffer, Client->FoV, FIXED_POINT(FOV));
		BitBufferSetSignedFixedPoint(&Buffer, Client->CameraX, FIXED_POINT(POS));
		BitBufferSetSignedFixedPoint(&Buffer, Client->CameraY, FIXED_POINT(POS));

		BitBufferContext EntitiesCount = BitBufferSave(&Buffer);
		BitBufferSkipBits(&Buffer, GAME_CONST_MAX_ENTITIES__BITS);

		EntityBits = calloc(TO_BYTES(GAME_CONST_MAX_ENTITIES), sizeof(uint8_t));
		AssertNotNull(EntityBits);

		EntitiesInViewCount = 0;

		QuadtreeQuery(&Quadtree, Client->CameraX, Client->CameraY, Client->CameraW, Client->CameraH);

		uint16_t* NewEntitiesInView = malloc(sizeof(*NewEntitiesInView) * EntitiesInViewCount);
		AssertNotNull(NewEntitiesInView);

		(void) memcpy(NewEntitiesInView, EntitiesInView, sizeof(*EntitiesInView) * EntitiesInViewCount);

		uint32_t NewEntitiesInViewCount = EntitiesInViewCount;

		uint16_t* EntityInView = Client->EntitiesInView;
		uint16_t* EntityInViewEnd = EntityInView + Client->EntitiesInViewCount;

		while(EntityInView != EntityInViewEnd)
		{
			if(!(EntityBits[*EntityInView >> 3] & (1 << (*EntityInView & 7))))
			{
				EntitiesInView[EntitiesInViewCount++] = *EntityInView;
			}

			++EntityInView;
		}

		free(Client->EntitiesInView);
		Client->EntitiesInView = NewEntitiesInView;
		Client->EntitiesInViewCount = NewEntitiesInViewCount;

		QuickSort(EntitiesInView, EntitiesInViewCount);

		EntityInView = EntitiesInView;
		EntityInViewEnd = EntityInView + EntitiesInViewCount;

		while(EntityInView != EntityInViewEnd)
		{
			uint8_t OldSet = !!(Client->EntityBits[*EntityInView >> 3] & (1 << (*EntityInView & 7)));
			uint8_t NewSet = !!(        EntityBits[*EntityInView >> 3] & (1 << (*EntityInView & 7)));

			BitBufferSetBits(&Buffer, OldSet, 1);
			BitBufferSetBits(&Buffer, NewSet, 1);

			QUADTREE_ENTITY* QTEntity = Quadtree.Entities + *EntityInView;
			GameEntity* Entity = Entities + *EntityInView;

			if(!OldSet)
			{
				/* Creation */

				AssertEQ(!!NewSet, 1);

				BitBufferSetBits(&Buffer, Entity->Type, ENTITY_TYPE__BITS);
				BitBufferSetBits(&Buffer, Entity->Subtype, TypeToSubtypeBits[Entity->Type]);

				BitBufferSetSignedFixedPoint(&Buffer, (QTEntity->X - Client->CameraX) * Client->FoV, FIXED_POINT(SCREEN_POS));
				BitBufferSetSignedFixedPoint(&Buffer, (QTEntity->Y - Client->CameraY) * Client->FoV, FIXED_POINT(SCREEN_POS));


				switch(Entity->Type)
				{

				case ENTITY_TYPE_TANK:
				{
					BitBufferSetFixedPoint(&Buffer, QTEntity->R, FIXED_POINT(RADIUS));

					break;
				}

				case ENTITY_TYPE_SHAPE:
				{
					break;
				}

				default: __builtin_unreachable();

				}


				switch(Entity->Type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					int WriteHP = Entity->HP != Entity->MaxHP;
					BitBufferSetBits(&Buffer, WriteHP, 1);

					BitBufferSetBits(&Buffer, Entity->TookDamage, 1);

					if(WriteHP)
					{
						BitBufferSetBits(&Buffer, Entity->HP, ShapeHPBits[Entity->Subtype]);
					}

					break;
				}

				default: __builtin_unreachable();

				}
			}
			else if(NewSet)
			{
				/* Update */

				switch(Entity->Type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					BitBufferSetSignedFixedPoint(&Buffer, (QTEntity->X - Client->CameraX) * Client->FoV, FIXED_POINT(SCREEN_POS));
					BitBufferSetSignedFixedPoint(&Buffer, (QTEntity->Y - Client->CameraY) * Client->FoV, FIXED_POINT(SCREEN_POS));

					BitBufferSetBits(&Buffer, Entity->UpdateHP, 1);
					BitBufferSetBits(&Buffer, Entity->TookDamage, 1);

					break;
				}

				default: __builtin_unreachable();

				}


				switch(Entity->Type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					if(Entity->UpdateHP)
					{
						BitBufferSetBits(&Buffer, Entity->HP, ShapeHPBits[Entity->Subtype]);
					}

					break;
				}

				default: __builtin_unreachable();

				}
			}
			else
			{
				/* Removal */
			}

			++EntityInView;
		}

		free(Client->EntityBits);
		Client->EntityBits = EntityBits;

		Buffer.Length = BitBufferGetConsumed(&Buffer);

		BitBufferRestore(&Buffer, &EntitiesCount);
		BitBufferSetBits(&Buffer, EntitiesInViewCount, GAME_CONST_MAX_ENTITIES__BITS);

		BitBufferRestore(&Buffer, &PacketLength);
		BitBufferSetBits(&Buffer, Buffer.Length, GAME_CONST_SERVER_PACKET_SIZE__BITS);

		ClientSend(&Buffer);
		free(Buffer.Buffer);
	}
}


int
main(
	int argc,
	char** argv
	)
{
	int Error;

	Entities = calloc(GAME_CONST_MAX_ENTITIES, sizeof(*Entities));
	AssertNotNull(Entities);

	Quadtree.X = 0;
	Quadtree.Y = 0;
	Quadtree.W = GAME_CONST_HALF_ARENA_SIZE + GAME_CONST_HALF_ARENA_CLEAR_ZONE;
	Quadtree.H = GAME_CONST_HALF_ARENA_SIZE + GAME_CONST_HALF_ARENA_CLEAR_ZONE;
	Quadtree.MinSize = GAME_CONST_MIN_QUADTREE_NODE_SIZE;
	Quadtree.Update = QuadtreeUpdateFN;
	Quadtree.IsColliding = QuadtreeIsCollidingFN;
	Quadtree.Collide = QuadtreeCollideFN;
	Quadtree.EntitiesSize = GAME_CONST_MAX_ENTITIES;
	QuadtreeInit(&Quadtree);

	Entities = calloc(Quadtree.EntitiesSize, sizeof(*Entities));
	AssertNotNull(Entities);

	for(int i = 0; i < 600; ++i)
	{
		SpawnShape(SHAPE_SQUARE);
	}

	for(int i = 0; i < 300; ++i)
	{
		SpawnShape(SHAPE_TRIANGLE);
	}

	for(int i = 0; i < 100; ++i)
	{
		SpawnShape(SHAPE_PENTAGON);
	}

	EntitiesInView = malloc(sizeof(*EntitiesInView) * GAME_CONST_MAX_ENTITIES);
	AssertNotNull(EntitiesInView);

	int EpollFD = epoll_create1(0);
	AssertNEQ(EpollFD, -1);

	struct epoll_event Events[GAME_CONST_MAX_PLAYERS];

	int ServerFD = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	AssertNEQ(ServerFD, -1);

	struct sockaddr_in6 Addr = {0};
	Addr.sin6_family = AF_INET6;
	Addr.sin6_addr = in6addr_any;
	Addr.sin6_port = htons(GAME_CONST_PORT);

	int ReceiveBufferSize = GAME_CONST_SERVER_RECV_SIZE;
	Error = setsockopt(ServerFD, SOL_SOCKET, SO_RCVBUF, &ReceiveBufferSize, sizeof(ReceiveBufferSize));
	AssertNEQ(Error, -1);

	int True = 1;
	Error = setsockopt(ServerFD, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(True));
	AssertNEQ(Error, -1);

	Error = setsockopt(ServerFD, SOL_SOCKET, SO_REUSEPORT, &True, sizeof(True));
	AssertNEQ(Error, -1);

	Error = bind(ServerFD, (struct sockaddr*) &Addr, sizeof(Addr));
	AssertNEQ(Error, -1);

	Error = listen(ServerFD, 64);
	AssertNEQ(Error, -1);

	struct timespec Wait;
	(void) clock_gettime(CLOCK_REALTIME, &Wait);

	LastTickAt = Wait.tv_nsec + Wait.tv_sec * 1000000000 - GAME_CONST_TICK_RATE_MS * 1000000;

	while(1)
	{
		++CurrentTick;

		struct timespec Time = {0};
		clock_gettime(CLOCK_REALTIME, &Time);
		CurrentTickAt = Time.tv_nsec + Time.tv_sec * 1000000000;

		int Count = epoll_wait(EpollFD, Events, GAME_CONST_MAX_PLAYERS, 0);

		struct epoll_event* Event = Events;
		struct epoll_event* EventEnd = Event + Count;

		for(; Event != EventEnd; ++Event)
		{
			uint32_t Flags = Event->events;
			Client = Event->data.ptr;

			if(Flags & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				ClientDestroy();

				close(Client->FD);
				RetClient();

				continue;
			}

			if(Flags & EPOLLIN)
			{
				ssize_t Bytes = read(Client->FD, Client->Buffer +
					Client->BufferUsed, ARRAYLEN(Client->Buffer) - Client->BufferUsed);

				if(Bytes >= 0)
				{
					Client->BufferUsed += Bytes;

					while(Client->BufferUsed)
					{
						uint32_t Read = ClientRead();

						if(Read == 0)
						{
							break;
						}

						Client->BufferUsed -= Read;

						(void) memmove(Client->Buffer, Client->Buffer + Read, Client->BufferUsed);
					}
				}
			}
		}

		while(ClientsUsed != GAME_CONST_MAX_PLAYERS || FreeClient != -1)
		{
			struct sockaddr_in6 Addr;
			socklen_t AddrLen = sizeof(Addr);
			int SocketFD = accept(ServerFD, (struct sockaddr*) &Addr, &AddrLen);

			if(SocketFD == -1)
			{
				AssertNEQ(errno, ENOMEM);
				AssertNEQ(errno, ENFILE);
				AssertNEQ(errno, EMFILE);
				AssertNEQ(errno, EINVAL);

				if(errno == EAGAIN)
				{
					break;
				}

				continue;
			}

			Client = GetClient();
			Client->FD = SocketFD;

			ClientCreate();

			(void) fcntl(SocketFD, F_SETFL, fcntl(SocketFD, F_GETFL, 0) | O_NONBLOCK);

			struct epoll_event Event =
			{
				.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP,
				.data =
				{
					.ptr = Client
				}
			};
			epoll_ctl(EpollFD, EPOLL_CTL_ADD, SocketFD, &Event);
		}

		GameUpdate();

		LastTickAt = CurrentTickAt;

		Wait.tv_nsec += GAME_CONST_TICK_RATE_MS * 1000000;
		if(Wait.tv_nsec >= 1000000000)
		{
			Wait.tv_nsec -= 1000000000;
			++Wait.tv_sec;
		}

		Error = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &Wait, NULL);
		AssertNEQ(Error, -1);
	}

	return 0;
}
