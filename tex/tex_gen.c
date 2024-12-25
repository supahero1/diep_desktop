#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define TEXT_FILE_SIZE (UINT32_C(1) << 24)


static int
qcompare(
	const void* A,
	const void* B
	)
{
	return strcmp(*((char**)A), *((char**)B));
}


int
main(
	void
	)
{
	char* TexHeaderBuffer = AllocMalloc(TEXT_FILE_SIZE);
	char* TexHeaderText = TexHeaderBuffer;
	AssertNotNull(TexHeaderText);

	char* TexSourceBuffer = AllocMalloc(TEXT_FILE_SIZE);
	char* TexSourceText = TexSourceBuffer;
	AssertNotNull(TexSourceText);

	char* TexResBuffer = AllocMalloc(TEXT_FILE_SIZE);
	char* TexResText = TexResBuffer;
	AssertNotNull(TexResText);


	TexHeaderText += sprintf(TexHeaderText, "#pragma once\n");
	TexHeaderText += sprintf(TexHeaderText, "\n#include <stdint.h>\n\n");

	TexSourceText += sprintf(TexSourceText, "#include <DiepDesktop/client/tex/base.h>\n");
	TexSourceText += sprintf(TexSourceText, "\n\nTexFile TextureFiles[TEXTURES_NUM] =\n{\n");

	struct dirent* Entry;
	DIR* Directory = opendir("tex/img");
	AssertNotNull(Directory);

	char Path[256];
	char FilePath[256];

	uint32_t TextureCount = 0;
	while((Entry = readdir(Directory)) != NULL)
	{
		if(Entry->d_name[0] == '.')
		{
			continue;
		}

		sprintf(Path, "tex/img/%s", Entry->d_name);

		struct stat Info;
		int Status = stat(Path, &Info);
		AssertNEQ(Status, -1);

		if((Info.st_mode & S_IFMT) != S_IFDIR || (((uint8_t) Entry->d_name[0]) - 48) > 9)
		{
			continue;
		}

		TexHeaderText += sprintf(TexHeaderText, "#include <DiepDesktop/client/tex/tex_%s.h>\n", Entry->d_name);
		TexSourceText += sprintf(TexSourceText, "\t{ %s.0f, \"%s.dds\" },\n", Entry->d_name, Entry->d_name);

		TexResText = TexResBuffer;
		TexResText += sprintf(TexResText, "#pragma once\n\n");

		char** FileNames = AllocMalloc(sizeof(char*) * TEXT_FILE_SIZE);
		AssertNotNull(FileNames);

		char** FileName = FileNames;

		struct dirent* TexEntry;
		DIR* TexDirectory = opendir(Path);

		while((TexEntry = readdir(TexDirectory)) != NULL)
		{
			sprintf(FilePath, "%s/%s", Path, TexEntry->d_name);

			struct stat TexInfo;
			Status = stat(FilePath, &TexInfo);
			AssertNEQ(Status, -1);

			if((TexInfo.st_mode & S_IFMT) == S_IFDIR)
			{
				continue;
			}

			*FileName = AllocMalloc(32);
			AssertNotNull(*FileName);

			char* TexName = *(FileName++);
			char* Char = TexEntry->d_name;
			uint8_t CurrentChar;

			while(1)
			{
				CurrentChar = *(Char++);

				if(CurrentChar == '.')
				{
					*TexName = '\0';

					break;
				}

				*(TexName++) = CurrentChar;
			}
		}

		closedir(TexDirectory);

		uint32_t Files = FileName - FileNames;
		qsort(FileNames, Files, sizeof(void*), qcompare);

		for(int i = 0; i < Files; ++i)
		{
			char* Char = FileNames[i];

			while(*Char)
			{
				uint8_t TempChar = *Char - 'a';
				uint8_t Bounds = 'z' - 'a';

				if(TempChar <= Bounds)
				{
					*Char ^= 0x20;
				}

				++Char;
			}

			TexResText += sprintf(TexResText,
				"#define TEXTURE_%s ((TexInfo){{ %d, %d }})\n",
				FileNames[i], TextureCount, i);
		}

		sprintf(Path, "include/DiepDesktop/client/tex/tex_%s.h", Entry->d_name);
		FileWrite(Path, (FileFile) {
			.Buffer = (void*) TexResBuffer,
			.Length = TexResText - TexResBuffer
		});

		++TextureCount;
	}

	closedir(Directory);


	TexHeaderText += sprintf(TexHeaderText, "\n#define TEXTURES_NUM %u\n", TextureCount);

	TexHeaderText += sprintf(TexHeaderText, "\n\ntypedef union TexInfo\n\{\n");
	TexHeaderText += sprintf(TexHeaderText, "\tuint16_t Indices[2];\n\n");
	TexHeaderText += sprintf(TexHeaderText, "\tstruct\n\t{\n");
	TexHeaderText += sprintf(TexHeaderText, "\t\tuint16_t Index;\n");
	TexHeaderText += sprintf(TexHeaderText, "\t\tuint16_t Layer;\n");
	TexHeaderText += sprintf(TexHeaderText, "\t}\n\tData;\n\n");
	TexHeaderText += sprintf(TexHeaderText, "\tstruct\n\t{\n");
	TexHeaderText += sprintf(TexHeaderText, "\t\tuint16_t Index;\n");
	TexHeaderText += sprintf(TexHeaderText, "\t\tuint16_t Layer;\n");
	TexHeaderText += sprintf(TexHeaderText, "\t};\n");
	TexHeaderText += sprintf(TexHeaderText, "}\nTexInfo;\n");

	TexHeaderText += sprintf(TexHeaderText, "\n\ntypedef struct TexFile\n\{\n");
	TexHeaderText += sprintf(TexHeaderText, "\tfloat Size;\n");
	TexHeaderText += sprintf(TexHeaderText, "\tconst char* Path;\n");
	TexHeaderText += sprintf(TexHeaderText, "}\nTexFile;\n");
	TexHeaderText += sprintf(TexHeaderText, "\n\nextern TexFile TextureFiles[TEXTURES_NUM];\n");
	TexHeaderText += sprintf(TexHeaderText, "\n\n#define TEXSIZE(Texture) (TextureFiles[(Texture).Index].Size)\n");

	FileWrite("include/DiepDesktop/client/tex/base.h", (FileFile) {
		.Buffer = (void*) TexHeaderBuffer,
		.Length = TexHeaderText - TexHeaderBuffer
	});


	TexSourceText += sprintf(TexSourceText, "};\n");

	FileWrite("src/client/tex/base.c", (FileFile) {
		.Buffer = (void*) TexSourceBuffer,
		.Length = TexSourceText - TexSourceBuffer
	});


	AllocFree(TEXT_FILE_SIZE, TexResBuffer);
	AllocFree(TEXT_FILE_SIZE, TexSourceBuffer);
	AllocFree(TEXT_FILE_SIZE, TexHeaderBuffer);

	return 0;
}
