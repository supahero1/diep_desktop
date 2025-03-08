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

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


bool
file_write(
	const char* path,
	file_t file
	)
{
	int fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(fd < 0)
	{
		return false;
	}

	if(ftruncate(fd, file.len) != 0)
	{
		return false;
	}

	if(lseek(fd, 0, SEEK_SET) != 0)
	{
		return false;
	}

	ssize_t bytes = write(fd, file.data, file.len);
	close(fd);

	return bytes == file.len;
}


bool
file_read_cap(
	const char* path,
	file_t* file,
	uint64_t cap
	)
{
	int fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		return false;
	}

	bool status = false;

	if(lseek(fd, 0, SEEK_SET) != 0)
	{
		goto goto_end;
	}

	struct stat Stat;
	if(fstat(fd, &Stat) != 0)
	{
		goto goto_end;
	}

	file->len = Stat.st_size;
	if(file->len > cap)
	{
		goto goto_end;
	}

	file->data = alloc_malloc(file->len);
	hard_assert_not_null(file->data);

	if(read(fd, file->data, file->len) != file->len)
	{
		file_free(*file);
	}
	else
	{
		status = true;
	}


	goto_end:

	close(fd);
	return status;
}


bool
file_read(
	const char* path,
	file_t* file
	)
{
	return file_read_cap(path, file, -1);
}


void
file_free(
	file_t file
	)
{
	alloc_free(file.data, file.len);
}
