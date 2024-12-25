#pragma once

#include <stdint.h>


typedef struct GamePacket
{
	uint8_t* Buffer;
	uint8_t* At;

	uintptr_t Length;
	uintptr_t Bit;
}
GamePacket;


typedef struct PacketContext
{
	uint8_t* At;
	uintptr_t Bit;
}
PacketContext;


extern void
WriteBits(
	GamePacket* Packet,
	uintptr_t Number,
	uintptr_t Bits
	);


extern uintptr_t
ReadBits(
	GamePacket* Packet,
	uintptr_t Bits
	);


extern uintptr_t
AvailableBits(
	GamePacket* Packet
	);


extern uintptr_t
GetConsumed(
	GamePacket* Packet
	);


extern void
SkipBits(
	GamePacket* Packet,
	uintptr_t Bits
	);


extern void
ResetBits(
	GamePacket* Packet
	);


extern PacketContext
SaveContext(
	GamePacket* Packet
	);


extern void
RestoreContext(
	GamePacket* Packet,
	const PacketContext* Context
	);


extern void
WriteFixedPoint(
	GamePacket* Packet,
	float Value,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	);


extern float
ReadFixedPoint(
	GamePacket* Packet,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	);


extern void
WriteSignedFixedPoint(
	GamePacket* Packet,
	float Value,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	);


extern float
ReadSignedFixedPoint(
	GamePacket* Packet,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	);
