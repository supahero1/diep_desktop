#pragma once

#include <stdint.h>


typedef enum DDSFlag
{
	DDS_FLAG_CAPS = 0x1,
	DDS_FLAG_HEIGHT = 0x2,
	DDS_FLAG_WIDTH = 0x4,
	DDS_FLAG_PITCH = 0x8,
	DDS_FLAG_PIXEL_FORMAT = 0x1000,
	DDS_FLAG_MIPMAP_COUNT = 0x20000,
	DDS_FLAG_LINEARSIZE = 0x80000,
	DDS_FLAG_DEPTH = 0x800000,
	kDDS_FLAG,

	DDS_REQUIRED_FLAGS = DDS_FLAG_CAPS | DDS_FLAG_HEIGHT |
		DDS_FLAG_WIDTH | DDS_FLAG_PIXEL_FORMAT | DDS_FLAG_LINEARSIZE
}
DDSFlag;

typedef enum DDSPixelFlag
{
	DDS_PIXEL_FLAG_ALPHAPIXELS = 0x1,
	DDS_PIXEL_FLAG_ALPHA = 0x2,
	DDS_PIXEL_FLAG_FOURCC = 0x4,
	DDS_PIXEL_FLAG_RGB = 0x40,
	DDS_PIXEL_FLAG_YUV = 0x200,
	DDS_PIXEL_FLAG_LUMINANCE = 0x20000,
	kDDS_PIXEL_FLAG,

	DDS_REQUIRED_PIXEL_FLAGS = DDS_PIXEL_FLAG_FOURCC
}
DDSPixelFlag;

typedef struct DDSPixelFormat
{
	uint32_t Size;
	uint32_t Flags;
	uint32_t FormatCode;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
}
DDSPixelFormat;

typedef struct DDSTexture
{
	uint32_t Magic;
	uint32_t Size;
	DDSFlag  Flags;
	uint32_t Height;
	uint32_t Width;
	uint32_t PitchOrLinearSize;
	uint32_t Depth;
	uint32_t MipMapCount;
	uint32_t Reserved1[11];
	DDSPixelFormat PixelFormat;
	uint32_t Caps;
	uint32_t Caps2;
	uint32_t Caps3;
	uint32_t Caps4;
	uint32_t Reserved2;
	uint32_t Format;
	uint32_t Dimension;
	uint32_t MiscFlag;
	uint32_t ArraySize;
	uint32_t MiscFlags2;
	uint8_t Data[];
}
DDSTexture;


extern DDSTexture*
DDSLoad(
	const char* Path
	);


extern void
DDSFree(
	DDSTexture* Texture
	);


extern uint64_t
DDSDataSize(
	DDSTexture* Texture
	);


extern uint64_t
DDSOffset(
	DDSTexture* Texture,
	uint32_t Layer
	);
