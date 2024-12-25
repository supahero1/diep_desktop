#include <DiepDesktop/client/dds.h>
#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <zstd.h>


extern DDSTexture*
DDSLoad(
	const char* Path
	)
{
	FileFile File;
	bool Status = FileRead(Path, &File);
	HardenedAssertTrue(Status);

	/* Decompress */
	uint64_t DecompressedSize = ZSTD_getFrameContentSize(File.Buffer, File.Length);
	HardenedAssertLT(DecompressedSize, /* 4GiB*/ 0x100000000);

	uint8_t* Content = AllocMalloc(DecompressedSize);
	HardenedAssertNotNull(Content);

	uint64_t RealSize = ZSTD_decompress(Content, DecompressedSize, File.Buffer, File.Length);
	HardenedAssertGE(RealSize, sizeof(DDSTexture));
	HardenedAssertLE(RealSize, DecompressedSize);

	FileFree(File);

	DDSTexture* Texture = (DDSTexture*) Content;
	HardenedAssertEQ(RealSize, sizeof(DDSTexture) + DDSDataSize(Texture));
	HardenedAssertEQ(Texture->Magic, 0x20534444);
	HardenedAssertEQ(Texture->Size, 124);
	HardenedAssertEQ((Texture->Flags & DDS_REQUIRED_FLAGS), DDS_REQUIRED_FLAGS);
	HardenedAssertEQ((Texture->PixelFormat.Flags & DDS_REQUIRED_PIXEL_FLAGS), DDS_REQUIRED_PIXEL_FLAGS);
	HardenedAssertEQ(Texture->PixelFormat.FormatCode, 0x30315844);
	HardenedAssertEQ(Texture->Dimension, 3);
	HardenedAssertEQ(Texture->Format, 72);
	HardenedAssertNEQ(Texture->ArraySize, 0);

	return Texture;
}


extern void
DDSFree(
	DDSTexture* Texture
	)
{
	AllocFree(sizeof(DDSTexture) + DDSDataSize(Texture), Texture);
}


uint64_t
DDSDataSize(
	DDSTexture* Texture
	)
{
	return DDSOffset(Texture, Texture->ArraySize);
}


uint64_t
DDSOffset(
	DDSTexture* Texture,
	uint32_t Layer
	)
{
	return Texture->PitchOrLinearSize * Layer;
}
