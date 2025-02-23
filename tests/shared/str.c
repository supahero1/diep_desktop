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

#include <DiepDesktop/shared/str.h>
#include <DiepDesktop/shared/debug.h>

#include <string.h>


void assert_used
test_should_pass__str_init_free(
	void
	)
{
	str_t str;
	str_init(&str);
	str_free(&str);
}


void assert_used
test_should_pass__str_init_cstr_copy(
	void
	)
{
	str_t str;
	str_init_cstr_copy(&str, (void*) "test");
	assert_false(strcmp((void*) str.str, "test"));
	str_free(&str);
}


void assert_used
test_should_pass__str_init_move(
	void
	)
{
	str_t str;
	str_init_cstr_copy(&str, (void*) "test");
	assert_false(strcmp((void*) str.str, "test"));

	str_t str2;
	str_init_move(&str2, &str);
	assert_false(strcmp((void*) str2.str, "test"));
	assert_true(str.str == NULL);
	str_free(&str2);
}


void assert_used
test_should_pass__str_init_move_empty(
	void
	)
{
	str_t str;
	str_init(&str);

	str_t str2;
	str_init_move(&str2, &str);
	assert_true(str.str == NULL);
	assert_true(str2.str == NULL);
	str_free(&str2);
}


void assert_used
test_should_pass__str_cmp(
	void
	)
{
	str_t str1 = { (void*) "test", 4 };
	str_t str2 = { (void*) "test", 4 };
	assert_true(str_cmp(&str1, &str2));

	str1 = (str_t) { (void*) "foo", 3 };
	str2 = (str_t) { (void*) "bar", 3 };
	assert_false(str_cmp(&str1, &str2));

	str1 = (str_t) { (void*) "foo", 3 };
	str2 = (str_t) { (void*) "foo", 4 };
	assert_false(str_cmp(&str1, &str2));

	str1 = (str_t) { (void*) "foo", 3 };
	str2 = (str_t) { (void*) "fOo", 3 };
	assert_false(str_cmp(&str1, &str2));
}


void assert_used
test_should_pass__str_case_cmp(
	void
	)
{
	str_t str1 = { (void*) "test", 4 };
	str_t str2 = { (void*) "test", 4 };
	assert_true(str_case_cmp(&str1, &str2));

	str1 = (str_t) { (void*) "foo", 3 };
	str2 = (str_t) { (void*) "bar", 3 };
	assert_false(str_case_cmp(&str1, &str2));

	str1 = (str_t) { (void*) "foo", 3 };
	str2 = (str_t) { (void*) "foo", 4 };
	assert_false(str_case_cmp(&str1, &str2));

	str1 = (str_t) { (void*) "foo", 3 };
	str2 = (str_t) { (void*) "fOo", 3 };
	assert_true(str_case_cmp(&str1, &str2));
}


void assert_used
test_should_fail__str_init_null(
	void
	)
{
	str_init(NULL);
}


void assert_used
test_should_fail__str_free_null(
	void
	)
{
	str_free(NULL);
}


void assert_used
test_should_fail__str_init_cstr_copy_null_str(
	void
	)
{
	str_init_cstr_copy(NULL, (void*) "test");
}


void assert_used
test_should_fail__str_init_cstr_copy_null_cstr(
	void
	)
{
	str_t str;
	str_init_cstr_copy(&str, NULL);
}


void assert_used
test_should_fail__str_init_cstr_copy_null(
	void
	)
{
	str_init_cstr_copy(NULL, NULL);
}


void assert_used
test_should_fail__str_init_cstr_move_null_str(
	void
	)
{
	str_init_cstr_move(NULL, (void*) "test");
}


void assert_used
test_should_fail__str_init_cstr_move_null_cstr(
	void
	)
{
	str_t str;
	str_init_cstr_move(&str, NULL);
}


void assert_used
test_should_fail__str_init_cstr_move_null(
	void
	)
{
	str_init_cstr_move(NULL, NULL);
}


void assert_used
test_should_fail__str_init_len_copy_null_str(
	void
	)
{
	str_init_len_copy(NULL, (void*) "test", 4);
}


void assert_used
test_should_pass__str_init_len_copy_null_cstr(
	void
	)
{
	str_t str;
	str_init_len_copy(&str, NULL, 0);
	str_free(&str);
}


void assert_used
test_should_fail__str_init_len_copy_null_cstr_non_zero_len(
	void
	)
{
	str_t str;
	str_init_len_copy(&str, NULL, 4);
}


void assert_used
test_should_fail__str_init_len_copy_null(
	void
	)
{
	str_init_len_copy(NULL, NULL, 4);
}


void assert_used
test_should_fail__str_init_len_move_null_str(
	void
	)
{
	str_init_len_move(NULL, (void*) "test", 4);
}


void assert_used
test_should_pass__str_init_len_move_null_cstr(
	void
	)
{
	str_t str;
	str_init_len_move(&str, NULL, 0);
	str_free(&str);
}


void assert_used
test_should_fail__str_init_len_move_null_cstr_non_zero_len(
	void
	)
{
	str_t str;
	str_init_len_move(&str, NULL, 4);
}


void assert_used
test_should_fail__str_init_len_move_null(
	void
	)
{
	str_init_len_move(NULL, NULL, 4);
}
