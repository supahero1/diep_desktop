#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/bit_buffer.h>

#include <math.h>


void
BitBufferSet(
	BitBuffer* Buffer,
	uint8_t* Data,
	uint64_t Length
	)
{
	Buffer->Buffer = Data;
	Buffer->At = Data;

	Buffer->Length = Length;
	Buffer->Bit = 0;
}


void
BitBufferReset(
	BitBuffer* Buffer
	)
{
	Buffer->At = Buffer->Buffer;
	Buffer->Bit = 0;
}


uint64_t
BitBufferGetAvailableBits(
	BitBuffer* Buffer
	)
{
	return ((Buffer->Length - (Buffer->At - Buffer->Buffer)) << 3) - Buffer->Bit;
}


uint64_t
BitBufferGetConsumed(
	BitBuffer* Buffer
	)
{
	return (Buffer->At - Buffer->Buffer) + !!Buffer->Bit;
}


void
BitBufferSkipBits(
	BitBuffer* Buffer,
	uint64_t Bits
	)
{
	Buffer->Bit += Bits;
	Buffer->At += Buffer->Bit >> 3;
	Buffer->Bit &= 7;
}


BitBufferContext
BitBufferSave(
	BitBuffer* Buffer
	)
{
	return
	(BitBufferContext)
	{
		.At = Buffer->At,
		.Bit = Buffer->Bit
	};
}


void
BitBufferRestore(
	BitBuffer* Buffer,
	const BitBufferContext* Context
	)
{
	Buffer->At = Context->At;
	Buffer->Bit = Context->Bit;
}


void
BitBufferSetBits(
	BitBuffer* Buffer,
	uint64_t Number,
	uint64_t Bits
	)
{
	Number &= ((uint64_t) 1 << Bits) - 1;

	uint8_t* At = Buffer->At;

	while(Bits)
	{
		uint64_t Delta = Bits - 8 + Buffer->Bit;

		if(Delta & 0x8000000000000000)
		{
			*At |= Number << -Delta;

			Buffer->Bit += Bits;

			break;
		}
		else
		{
			*(At++) |= Number >> Delta;

			Buffer->Bit = 0;
			Bits = Delta;
		}
	}

	Buffer->At = At;
}


uint64_t
BitBufferGetBits(
	BitBuffer* Buffer,
	uint64_t Bits
	)
{
	uint64_t Mask = ((uint64_t) 1 << Bits) - 1;
	uint64_t Number = 0;

	uint8_t* At = Buffer->At;

	while(Bits)
	{
		uint64_t Delta = Bits - 8 + Buffer->Bit;

		if(Delta & 0x8000000000000000)
		{
			Number |= *At >> -Delta;

			Buffer->Bit += Bits;

			break;
		}
		else
		{
			Number |= (uint64_t) *(At++) << Delta;

			Buffer->Bit = 0;
			Bits = Delta;
		}
	}

	Buffer->At = At;

	Number &= Mask;

	return Number;
}


extern void
BitBufferSetBitsVar(
	BitBuffer* Buffer,
	uint64_t Number,
	uint64_t Bits,
	uint64_t Segment
	)
{
	uint64_t SegmentBit = (uint64_t) 1 << Segment;
	uint64_t SegmentMask = SegmentBit - 1;

	while(Bits >= Segment)
	{
		BitBufferSetBits(Buffer, (Number & SegmentMask) | SegmentBit, Segment + 1);

		Number >>= Segment;
		Bits -= Segment;
	}

	if(Bits)
	{
		BitBufferSetBits(Buffer, Number, Segment + 1);
	}
}


extern uint64_t
BitBufferGetBitsVar(
	BitBuffer* Buffer,
	uint64_t Segment
	)
{
	uint64_t Number = 0;
	uint64_t Shift = 0;

	uint64_t SegmentBit = (uint64_t) 1 << Segment;
	uint64_t SegmentMask = SegmentBit - 1;

	while(1)
	{
		uint64_t Part = BitBufferGetBits(Buffer, Segment + 1);

		Number |= (Part & SegmentMask) << Shift;

		if(!(Part & SegmentBit))
		{
			break;
		}

		Shift += Segment;
	}

	return Number;
}


void
BitBufferSetFixedPoint(
	BitBuffer* Buffer,
	float Value,
	uint64_t IntegerBits,
	uint64_t FractionBits
	)
{
	BitBufferSetBits(Buffer, ROUNDF(Value * UINT_TO_FLOAT((FractionBits + 127) << 23)), IntegerBits + FractionBits);
}


float
BitBufferGetFixedPoint(
	BitBuffer* Buffer,
	uint64_t IntegerBits,
	uint64_t FractionBits
	)
{
	return BitBufferGetBits(Buffer, IntegerBits + FractionBits) * UINT_TO_FLOAT((-FractionBits + 127) << 23);
}


void
BitBufferSetSignedFixedPoint(
	BitBuffer* Buffer,
	float Value,
	uint64_t IntegerBits,
	uint64_t FractionBits
	)
{
	uint32_t UValue = FLOAT_TO_UINT(Value);
	BitBufferSetBits(Buffer, UValue >> 31, 1);
	Value = UINT_TO_FLOAT(UValue & 0x7FFFFFFF);
	BitBufferSetFixedPoint(Buffer, Value, IntegerBits, FractionBits);
}


float
BitBufferGetSignedFixedPoint(
	BitBuffer* Buffer,
	uint64_t IntegerBits,
	uint64_t FractionBits
	)
{
	uint64_t Sign = BitBufferGetBits(Buffer, 1);
	uint32_t Value = FLOAT_TO_UINT(BitBufferGetFixedPoint(Buffer, IntegerBits, FractionBits));
	Value |= Sign << 31;
	return UINT_TO_FLOAT(Value);
}
