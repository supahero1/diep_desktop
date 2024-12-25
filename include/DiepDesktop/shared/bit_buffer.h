#pragma once

#include <stdint.h>


typedef struct BitBuffer
{
	uint8_t* Buffer;
	uint8_t* At;

	uint64_t Length;
	uint64_t Bit;
}
BitBuffer;


typedef struct BitBufferContext
{
	uint8_t* At;
	uint64_t Bit;
}
BitBufferContext;


extern void
BitBufferSet(
	BitBuffer* Buffer,
	uint8_t* Data,
	uint64_t Length
	);


extern void
BitBufferReset(
	BitBuffer* Buffer
	);


extern uint64_t
BitBufferGetAvailableBits(
	BitBuffer* Buffer
	);


extern uint64_t
BitBufferGetConsumed(
	BitBuffer* Buffer
	);


extern void
BitBufferSkipBits(
	BitBuffer* Buffer,
	uint64_t Bits
	);


extern BitBufferContext
BitBufferSave(
	BitBuffer* Buffer
	);


extern void
BitBufferRestore(
	BitBuffer* Buffer,
	const BitBufferContext* Context
	);


extern void
BitBufferSetBits(
	BitBuffer* Buffer,
	uint64_t Number,
	uint64_t Bits
	);


extern uint64_t
BitBufferGetBits(
	BitBuffer* Buffer,
	uint64_t Bits
	);


extern void
BitBufferSetBitsVar(
	BitBuffer* Buffer,
	uint64_t Number,
	uint64_t Bits,
	uint64_t Segment
	);


extern uint64_t
BitBufferGetBitsVar(
	BitBuffer* Buffer,
	uint64_t Segment
	);


extern void
BitBufferSetFixedPoint(
	BitBuffer* Buffer,
	float Value,
	uint64_t IntegerBits,
	uint64_t FractionBits
	);


extern float
BitBufferGetFixedPoint(
	BitBuffer* Buffer,
	uint64_t IntegerBits,
	uint64_t FractionBits
	);


extern void
BitBufferSetSignedFixedPoint(
	BitBuffer* Buffer,
	float Value,
	uint64_t IntegerBits,
	uint64_t FractionBits
	);


extern float
BitBufferGetSignedFixedPoint(
	BitBuffer* Buffer,
	uint64_t IntegerBits,
	uint64_t FractionBits
	);
