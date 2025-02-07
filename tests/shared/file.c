/*
 *   Copyright 2025 Franciszek Balcerak
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

#include <string.h>


uint8_t data[] = "test\n";
uint64_t len = sizeof(data) - 1;


void assert_used
test_should_pass__file_read(
	void
	)
{
	file_t file;
	bool status = file_read("tests/shared/file_read.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void assert_used
test_should_pass__file_read_non_existent(
	void
	)
{
	file_t file;
	bool status = file_read("tests/shared/file_read_non_existent.txt", &file);
	assert_false(status);
}


void assert_used
test_should_pass__file_read_cap(
	void
	)
{
	file_t file;
	bool status = file_read_cap("tests/shared/file_read.txt", &file, len - 1);
	assert_false(status);
}


void assert_used
test_should_pass__file_read_cap_non_existent(
	void
	)
{
	file_t file;
	bool status = file_read_cap("tests/shared/file_read_non_existent.txt", &file, len - 1);
	assert_false(status);
}


void assert_used
test_should_pass__file_write(
	void
	)
{
	file_t file =
	{
		.data = data,
		.len = len
	};
	bool status = file_write("bin/tests/shared/file_write.txt", file);
	assert_true(status);

	status = file_read("bin/tests/shared/file_write.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void assert_used
test_should_pass__file_read_multiple_should_not_move_cursor(
	void
	)
{
	file_t file;
	bool status = file_read("tests/shared/file_read.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	status = file_read("tests/shared/file_read.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void assert_used
test_should_pass__file_write_multiple_should_not_append(
	void
	)
{
	file_t file =
	{
		.data = data,
		.len = len
	};
	bool status = file_write("bin/tests/shared/file_write.txt", file);
	assert_true(status);

	status = file_write("bin/tests/shared/file_write.txt", file);
	assert_true(status);

	status = file_read("bin/tests/shared/file_write.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}
