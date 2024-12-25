#include <DiepDesktop/shared/io.h>
#include <DiepDesktop/shared/base.h>

#include <math.h>


void
WriteBits(
	GamePacket* Packet,
	uintptr_t Number,
	uintptr_t Bits
	)
{
	if(!Bits)
	{
		return;
	}

	Number &= ((uintptr_t) 1 << Bits) - 1;

	uint8_t* At = Packet->At;

	while(Bits)
	{
		uintptr_t Delta = Bits - 8 + Packet->Bit;

		if(Delta & 0x8000000000000000)
		{
			*At |= Number << -Delta;

			Packet->Bit += Bits;

			break;
		}
		else
		{
			*(At++) |= Number >> Delta;

			Packet->Bit = 0;
			Bits = Delta;
		}
	}

	Packet->At = At;
}


uintptr_t
ReadBits(
	GamePacket* Packet,
	uintptr_t Bits
	)
{
	if(!Bits)
	{
		return 0;
	}

	uintptr_t Mask = ((uintptr_t) 1 << Bits) - 1;
	uintptr_t Number = 0;

	uint8_t* At = Packet->At;

	while(Bits)
	{
		uintptr_t Delta = Bits - 8 + Packet->Bit;

		if(Delta & 0x8000000000000000)
		{
			Number |= *At >> -Delta;

			Packet->Bit += Bits;

			break;
		}
		else
		{
			Number |= (uintptr_t) *(At++) << Delta;

			Packet->Bit = 0;
			Bits = Delta;
		}
	}

	Packet->At = At;

	Number &= Mask;

	return Number;
}


uintptr_t
AvailableBits(
	GamePacket* Packet
	)
{
	return ((Packet->Length - (Packet->At - Packet->Buffer)) << 3) - Packet->Bit;
}


uintptr_t
GetConsumed(
	GamePacket* Packet
	)
{
	return (Packet->At - Packet->Buffer) + !!Packet->Bit;
}


void
SkipBits(
	GamePacket* Packet,
	uintptr_t Bits
	)
{
	Packet->Bit += Bits;
	Packet->At += Packet->Bit >> 3;
	Packet->Bit &= 7;
}


void
ResetBits(
	GamePacket* Packet
	)
{
	Packet->At = Packet->Buffer;
	Packet->Bit = 0;
}


PacketContext
SaveContext(
	GamePacket* Packet
	)
{
	return
	(PacketContext)
	{
		.At = Packet->At,
		.Bit = Packet->Bit
	};
}


void
RestoreContext(
	GamePacket* Packet,
	const PacketContext* Context
	)
{
	Packet->At = Context->At;
	Packet->Bit = Context->Bit;
}


void
WriteFixedPoint(
	GamePacket* Packet,
	float Value,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	)
{
	WriteBits(Packet, ROUNDF(Value * UINT_TO_FLOAT((FractionBits + 127) << 23)), IntegerBits + FractionBits);
}


float
ReadFixedPoint(
	GamePacket* Packet,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	)
{
	return ReadBits(Packet, IntegerBits + FractionBits) * UINT_TO_FLOAT((-FractionBits + 127) << 23);
}


void
WriteSignedFixedPoint(
	GamePacket* Packet,
	float Value,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	)
{
	uint32_t UValue = FLOAT_TO_UINT(Value);
	WriteBits(Packet, UValue >> 31, 1);
	Value = UINT_TO_FLOAT(UValue & 0x7FFFFFFF);
	WriteFixedPoint(Packet, Value, IntegerBits, FractionBits);
}


float
ReadSignedFixedPoint(
	GamePacket* Packet,
	uintptr_t IntegerBits,
	uintptr_t FractionBits
	)
{
	uintptr_t Sign = ReadBits(Packet, 1);
	uint32_t Value = FLOAT_TO_UINT(ReadFixedPoint(Packet, IntegerBits, FractionBits));
	Value |= Sign << 31;
	return UINT_TO_FLOAT(Value);
}
