#pragma once

#include <stdint.h>


typedef struct FileFile
{
	uint64_t Length;
	uint8_t* Buffer;
}
FileFile;


extern bool
FileWrite(
	const char* Path,
	FileFile File
	);


extern bool
FileRead(
	const char* Path,
	FileFile* File
	);


extern void
FileFree(
	FileFile File
	);
