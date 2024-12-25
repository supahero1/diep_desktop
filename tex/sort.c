#include <DiepDesktop/shared/debug.h>

#include <png.h>
#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


static uint32_t
SizeofPNG(
	const char* Filename
	)
{
	FILE* File = fopen(Filename, "rb");
	AssertNotNull(File);

	png_structp PNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	AssertNotNull(PNG);

	png_infop Info = png_create_info_struct(PNG);
	AssertNotNull(Info);

	png_init_io(PNG, File);
	png_read_info(PNG, Info);

	uint32_t Size = png_get_image_width(PNG, Info);

	png_destroy_read_struct(&PNG, &Info, NULL);
	fclose(File);

	return Size;
}


int
main(
	void
	)
{
	struct dirent* Entry;
	DIR* Directory = opendir("tex/img");
	AssertNotNull(Directory);

	char OldPath[256];
	char DirPath[256];
	char NewPath[256];
	int Sizes[16] = {0};

	while((Entry = readdir(Directory)) != NULL)
	{
		if(!strstr(Entry->d_name, ".png"))
		{
			continue;
		}

		sprintf(OldPath, "tex/img/%s", Entry->d_name);

		uint32_t Size = SizeofPNG(OldPath);

		uint32_t SizePO2 = __builtin_ctz(Size);
		if(Sizes[SizePO2] == 0)
		{
			Sizes[SizePO2] = 1;

			sprintf(DirPath, "tex/img/%u", Size);
#ifdef _WIN32
			int Status = mkdir(DirPath);
#else
			int Status = mkdir(DirPath, 0755);
#endif
			AssertEQ(Status, 0);
		}

		sprintf(NewPath, "tex/img/%u/%s", Size, Entry->d_name);
		rename(OldPath, NewPath);
	}

	closedir(Directory);

	return 0;
}
