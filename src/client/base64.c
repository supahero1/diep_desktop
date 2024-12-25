#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/client/base64.h>
#include <DiepDesktop/shared/alloc_ext.h>


Static uint64_t
Base64EncodedLength(
	uint64_t Length
	)
{
	return ((Length + 2) / 3) << 2;
}


Static uint8_t
Base64EncodeTable[] =
{ /* Devious */
	'y', 'n', 'X', 'P', 'Q', 'S', 'W', 'e',
	'U', 't', 'k', 'G', '2', '8', 'O', 'D',
	'K', 'b', '6', 'T', 'o', '0', 'x', '5',
	'N', 'Y', '7', '/', 'j', 'l', 'q', 'v',
	'9', 'a', 's', 'H', 'C', 'i', 'M', 'g',
	'B', 'J', 'E', 'd', '1', 'w', 'p', 'F',
	'Z', 'I', 'r', 'R', 'z', 'u', 'm', 'h',
	'3', 'A', 'V', '4', 'f', 'c', 'L', '+'
};


uint8_t*
Base64Encode(
	const uint8_t* Data,
	uint64_t InLength,
	uint64_t* OutLength
	)
{
	uint64_t EncodedLength = Base64EncodedLength(InLength);
	uint8_t* Encoded = AllocMalloc(EncodedLength);
	AssertNotNull(Encoded);

	uint64_t FullEncodes = InLength / 3;
	const uint8_t* DataEnd = Data + FullEncodes * 3;

	while(Data != DataEnd)
	{
		*(Encoded++) = Base64EncodeTable[*Data >> 2];
		*(Encoded++) = Base64EncodeTable[((*Data & 0x03) << 4) | (*(Data + 1) >> 4)];
		++Data;
		*(Encoded++) = Base64EncodeTable[((*Data & 0x0F) << 2) | (*(Data + 1) >> 6)];
		++Data;
		*(Encoded++) = Base64EncodeTable[*Data & 0x3F];
		++Data;
	}


	switch(InLength % 3)
	{

	case 0: break;

	case 1:
	{
		*(Encoded++) = Base64EncodeTable[*Data >> 2];
		*(Encoded++) = Base64EncodeTable[(*Data & 0x03) << 4];
		*(Encoded++) = '=';
		*(Encoded++) = '=';
		break;
	}

	case 2:
	{
		*(Encoded++) = Base64EncodeTable[*Data >> 2];
		*(Encoded++) = Base64EncodeTable[((*Data & 0x03) << 4) | (*(Data + 1) >> 4)];
		++Data;
		*(Encoded++) = Base64EncodeTable[(*Data & 0x0F) << 2];
		*(Encoded++) = '=';
		break;
	}

	default: AssertUnreachable();

	}


	if(OutLength != NULL)
	{
		*OutLength = EncodedLength;
	}

	return Encoded - EncodedLength;
}


Static uint64_t
Base64DecodedLength(
	uint64_t Length
	)
{
	return (Length * 3) >> 2;
}


Static uint8_t
Base64DecodeTable[256] =
{
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0, 63,  0,  0,  0, 27,
	21, 44, 12, 56, 59, 23, 18, 26,
	13, 32,  0,  0,  0,  0,  0,  0,
	 0, 57, 40, 36, 15, 42, 47, 11,
	35, 49, 41, 16, 62, 38, 24, 14,
	 3,  4, 51,  5, 19,  8, 58,  6,
	 2, 25, 48,  0,  0,  0,  0,  0,
	 0, 33, 17, 61, 43,  7, 60, 39,
	55, 37, 28, 10, 29, 54,  1, 20,
	46, 30, 50, 34,  9, 53, 31, 45,
	22,  0, 52,  0,  0,  0,  0,  0,
};


uint8_t*
Base64Decode(
	const uint8_t* Data,
	uint64_t InLength,
	uint64_t* OutLength
	)
{
	if(InLength == 0 || (InLength & 3) != 0)
	{
		return NULL;
	}

	uint64_t DecodedLength = Base64DecodedLength(InLength);
	uint8_t* Decoded = AllocMalloc(DecodedLength);
	AssertNotNull(Decoded);

	const uint8_t* DataEnd = Data + InLength;

	while(Data != DataEnd)
	{
		*(Decoded++) = (Base64DecodeTable[*Data] << 2) | (Base64DecodeTable[*(Data + 1)] >> 4);
		++Data;
		*(Decoded++) = (Base64DecodeTable[*Data] << 4) | (Base64DecodeTable[*(Data + 1)] >> 2);
		++Data;
		*(Decoded++) = (Base64DecodeTable[*Data] << 6) | Base64DecodeTable[*(Data + 1)];
		Data += 2;
	}

	while(*(Data - 1) == '=')
	{
		--Data;
		--Decoded;
		--DecodedLength;
	}

	if(OutLength != NULL)
	{
		*OutLength = DecodedLength;
	}

	return Decoded - DecodedLength;
}
