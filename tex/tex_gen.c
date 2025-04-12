/*
 *   Copyright 2024-2025 Franciszek Balcerak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#define TEXT_FILE_SIZE (UINT32_C(1) << 24)


private int
sort_compare(
	const void* a,
	const void* b
	)
{
	return strcmp(*((char**) a), *((char**) b));
}


int
main(
	void
	)
{
	char* tex_h_data = alloc_malloc(TEXT_FILE_SIZE);
	char* tex_h_str = tex_h_data;
	assert_not_null(tex_h_str);

	char* tex_c_data = alloc_malloc(TEXT_FILE_SIZE);
	char* tex_c_str = tex_c_data;
	assert_not_null(tex_c_str);

	char* tex_data = alloc_malloc(TEXT_FILE_SIZE);
	char* tex_str = tex_data;
	assert_not_null(tex_str);


	tex_h_str += sprintf(tex_h_str, "/* This file was generated by tex_gen.c */\n\n");
	tex_h_str += sprintf(tex_h_str, "#pragma once\n\n");
	tex_h_str += sprintf(tex_h_str, "#include <stdint.h>\n\n");

	tex_c_str += sprintf(tex_c_str, "/* This file was generated by tex_gen.c */\n\n");
	tex_c_str += sprintf(tex_c_str, "#include <DiepDesktop/client/tex/base.h>\n\n\n");
	tex_c_str += sprintf(tex_c_str, "tex_file_t tex_files[TEX__COUNT] =\n{\n");

	struct dirent* entry;
	DIR* dir = opendir("tex/img");
	assert_not_null(dir);

	char path[512];
	char temp_path[768];

	uint32_t tex_count = 0;

	while((entry = readdir(dir)) != NULL)
	{
		if(entry->d_name[0] == '.')
		{
			continue;
		}

		sprintf(path, "tex/img/%s", entry->d_name);

		struct stat info;
		int status = stat(path, &info);
		assert_eq(status, 0);

		if((info.st_mode & S_IFMT) != S_IFDIR || (((uint8_t) entry->d_name[0]) - 48) > 9)
		{
			continue;
		}

		tex_h_str += sprintf(tex_h_str, "#include <DiepDesktop/client/tex/tex_%s.h>\n", entry->d_name);
		tex_c_str += sprintf(tex_c_str, "\t{ %s, \"%s.dds\" },\n", entry->d_name, entry->d_name);

		tex_str = tex_data;
		tex_str += sprintf(tex_str, "/* This file was generated by tex_gen.c */\n\n");
		tex_str += sprintf(tex_str, "#pragma once\n\n");

		char** filenames = alloc_malloc(sizeof(char*) * TEXT_FILE_SIZE);
		assert_not_null(filenames);

		char** filename = filenames;

		struct dirent* tex_entry;
		DIR* tex_dir = opendir(path);

		while((tex_entry = readdir(tex_dir)) != NULL)
		{
			sprintf(temp_path, "%s/%s", path, tex_entry->d_name);

			struct stat tex_file;
			status = stat(temp_path, &tex_file);
			assert_neq(status, -1);

			if((tex_file.st_mode & S_IFMT) == S_IFDIR)
			{
				continue;
			}

			*filename = alloc_malloc(32);
			assert_not_null(*filename);

			char* tex_name = *(filename++);
			char* cur = tex_entry->d_name;
			uint8_t byte;

			while(1)
			{
				byte = *(cur++);
				if(byte == '.')
				{
					*tex_name = '\0';
					break;
				}

				*(tex_name++) = byte;
			}
		}

		closedir(tex_dir);

		uint32_t files = filename - filenames;
		qsort(filenames, files, sizeof(void*), sort_compare);

		for(int i = 0; i < files; ++i)
		{
			char* cur = filenames[i];
			while(*cur)
			{
				uint8_t byte = *cur - 'a';
				uint8_t max_byte = 'z' - 'a';

				if(byte <= max_byte)
				{
					*cur ^= 0x20;
				}

				++cur;
			}

			tex_str += sprintf(tex_str,
				"#define TEX_%-16s ((tex_t){ %2d, %4d })\n",
				filenames[i], tex_count, i);
		}

		sprintf(path, "include/DiepDesktop/client/tex/tex_%s.h", entry->d_name);

		file_t tex_h =
		{
			.data = (void*) tex_data,
			.len = tex_str - tex_data
		};
		file_write(path, tex_h);

		++tex_count;
	}

	closedir(dir);

	alloc_free(tex_data, TEXT_FILE_SIZE);


	tex_c_str += sprintf(tex_c_str, "};\n");

	file_t base_c =
	{
		.data = (void*) tex_c_data,
		.len = tex_c_str - tex_c_data
	};
	file_write("src/client/tex/base.c", base_c);

	alloc_free(tex_c_data, TEXT_FILE_SIZE);


	tex_h_str += sprintf(tex_h_str, "\n");
	tex_h_str += sprintf(tex_h_str, "#define TEX__COUNT %u\n", tex_count);
	tex_h_str += sprintf(tex_h_str, "#define TEX__SIZE(tex) (tex_files[(tex).idx].size)\n\n\n");
	tex_h_str += sprintf(tex_h_str, "typedef struct tex\n{\n");
	tex_h_str += sprintf(tex_h_str, "\tuint16_t idx;\n");
	tex_h_str += sprintf(tex_h_str, "\tuint16_t layer;\n");
	tex_h_str += sprintf(tex_h_str, "}\ntex_t;\n\n\n");
	tex_h_str += sprintf(tex_h_str, "typedef struct tex_file\n\{\n");
	tex_h_str += sprintf(tex_h_str, "\tfloat size;\n");
	tex_h_str += sprintf(tex_h_str, "\tconst char* path;\n");
	tex_h_str += sprintf(tex_h_str, "}\ntex_file_t;\n\n\n");
	tex_h_str += sprintf(tex_h_str, "extern tex_file_t tex_files[TEX__COUNT];\n");

	file_t base_h =
	{
		.data = (void*) tex_h_data,
		.len = tex_h_str - tex_h_data
	};
	file_write("include/DiepDesktop/client/tex/base.h", base_h);

	alloc_free(tex_h_data, TEXT_FILE_SIZE);

	return 0;
}
