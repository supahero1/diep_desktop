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
test_should_pass__str_init_copy_cstr(
	void
	)
{
	str_t str;
	str_init_copy_cstr(&str, "test");

	assert_true(str_cmp_cstr(&str, "test"));

	str_free(&str);
}


void assert_used
test_should_pass__str_init_move(
	void
	)
{
	str_t str;
	str_init_copy_cstr(&str, "test");

	assert_true(str_cmp_cstr(&str, "test"));

	str_t str2;
	str_init_move(&str2, &str);

	assert_true(str_cmp_cstr(&str2, "test"));
	assert_true(str_is_empty(&str));

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

	assert_true(str_is_empty(&str));
	assert_true(str_is_empty(&str2));

	str_free(&str2);
}


void assert_used
test_should_pass__str_set_copy_cstr(
	void
	)
{
	str_t str;
	str_init(&str);

	str_set_copy_cstr(&str, "test");
	assert_true(str_cmp_cstr(&str, "test"));

	str_set_copy_cstr(&str, "test2");
	assert_true(str_cmp_cstr(&str, "test2"));

	str_free(&str);
}


void assert_used
test_should_pass__str_set_move_cstr(
	void
	)
{
	str_t str;
	str_init_copy_cstr(&str, "test");

	str_t str2;
	str_init(&str2);

	str_set_move_cstr(&str2, str.str);
	str_init(&str);

	assert_true(str_cmp_cstr(&str2, "test"));

	str_free(&str2);
}


void assert_used
test_should_pass__str_set_copy(
	void
	)
{
	str_t str;
	str_init_copy_cstr(&str, "test");

	str_t str2;
	str_init(&str2);

	str_set_copy(&str2, &str);
	assert_true(str_cmp_cstr(&str2, "test"));

	str_free(&str);
	str_free(&str2);
}


void assert_used
test_should_pass__str_set_move(
	void
	)
{
	str_t str;
	str_init_copy_cstr(&str, "test");

	str_t str2;
	str_init(&str2);

	str_set_move(&str2, &str);
	assert_true(str_cmp_cstr(&str2, "test"));
	assert_true(str_is_empty(&str));

	str_free(&str2);
}


void assert_used
test_should_pass__str_cmp(
	void
	)
{
	str_t str1 = { "test", 4 };
	str_t str2 = { "test", 4 };
	assert_true(str_cmp(&str1, &str2));

	str1 = (str_t) { "foo", 3 };
	str2 = (str_t) { "bar", 3 };
	assert_false(str_cmp(&str1, &str2));

	str1 = (str_t) { "foo", 3 };
	str2 = (str_t) { "foo", 4 };
	assert_false(str_cmp(&str1, &str2));

	str1 = (str_t) { "foo", 3 };
	str2 = (str_t) { "fOo", 3 };
	assert_false(str_cmp(&str1, &str2));
}


void assert_used
test_should_pass__str_case_cmp(
	void
	)
{
	str_t str1 = { "test", 4 };
	str_t str2 = { "test", 4 };
	assert_true(str_case_cmp(&str1, &str2));

	str1 = (str_t) { "foo", 3 };
	str2 = (str_t) { "bar", 3 };
	assert_false(str_case_cmp(&str1, &str2));

	str1 = (str_t) { "foo", 3 };
	str2 = (str_t) { "foo", 4 };
	assert_false(str_case_cmp(&str1, &str2));

	str1 = (str_t) { "foo", 3 };
	str2 = (str_t) { "fOo", 3 };
	assert_true(str_case_cmp(&str1, &str2));
}


void assert_used
test_should_pass__str_cmp_cstr(
	void
	)
{
	str_t str = { "test", 4 };
	assert_true(str_cmp_cstr(&str, "test"));

	str = (str_t) { "foo", 3 };
	assert_false(str_cmp_cstr(&str, "bar"));

	str = (str_t) { "foo", 3 };
	assert_false(str_cmp_cstr(&str, "foo2"));

	str = (str_t) { "foo", 3 };
	assert_false(str_cmp_cstr(&str, "fOo"));
}


void assert_used
test_should_pass__str_case_cmp_cstr(
	void
	)
{
	str_t str = { "test", 4 };
	assert_true(str_case_cmp_cstr(&str, "test"));

	str = (str_t) { "foo", 3 };
	assert_false(str_case_cmp_cstr(&str, "bar"));

	str = (str_t) { "foo", 3 };
	assert_false(str_case_cmp_cstr(&str, "foo2"));

	str = (str_t) { "foo", 3 };
	assert_true(str_case_cmp_cstr(&str, "fOo"));
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
test_should_fail__str_init_copy_cstr_null_str(
	void
	)
{
	str_init_copy_cstr(NULL, "test");
}


void assert_used
test_should_fail__str_init_copy_cstr_null_cstr(
	void
	)
{
	str_t str;
	str_init_copy_cstr(&str, NULL);
}


void assert_used
test_should_fail__str_init_copy_cstr_null(
	void
	)
{
	str_init_copy_cstr(NULL, NULL);
}


void assert_used
test_should_fail__str_init_move_cstr_null_str(
	void
	)
{
	str_init_move_cstr(NULL, "test");
}


void assert_used
test_should_fail__str_init_move_cstr_null_cstr(
	void
	)
{
	str_t str;
	str_init_move_cstr(&str, NULL);
}


void assert_used
test_should_fail__str_init_move_cstr_null(
	void
	)
{
	str_init_move_cstr(NULL, NULL);
}


void assert_used
test_should_fail__str_init_copy_len_null_str(
	void
	)
{
	str_init_copy_len(NULL, "test", 4);
}


void assert_used
test_should_pass__str_init_copy_len_null_cstr(
	void
	)
{
	str_t str;
	str_init_copy_len(&str, NULL, 0);
	str_free(&str);
}


void assert_used
test_should_fail__str_init_copy_len_null_cstr_non_zero_len(
	void
	)
{
	str_t str;
	str_init_copy_len(&str, NULL, 4);
}


void assert_used
test_should_fail__str_init_copy_len_null(
	void
	)
{
	str_init_copy_len(NULL, NULL, 4);
}


void assert_used
test_should_fail__str_init_move_len_null_str(
	void
	)
{
	str_init_move_len(NULL, "test", 4);
}


void assert_used
test_should_pass__str_init_move_len_null_cstr(
	void
	)
{
	str_t str;
	str_init_move_len(&str, NULL, 0);
	str_free(&str);
}


void assert_used
test_should_fail__str_init_move_len_null_cstr_non_zero_len(
	void
	)
{
	str_t str;
	str_init_move_len(&str, NULL, 4);
}


void assert_used
test_should_fail__str_init_move_len_null(
	void
	)
{
	str_init_move_len(NULL, NULL, 4);
}


void assert_used
test_should_fail__str_init_copy_null_other(
	void
	)
{
	str_t str;
	str_init_copy(&str, NULL);
}


void assert_used
test_should_fail__str_init_copy_null_str(
	void
	)
{
	str_t str;
	str_init(&str);
	str_init_copy(NULL, &str);
}


void assert_used
test_should_fail__str_init_copy_null(
	void
	)
{
	str_init_copy(NULL, NULL);
}


void assert_used
test_should_fail__str_init_move_null_other(
	void
	)
{
	str_t str;
	str_init_move(&str, NULL);
}


void assert_used
test_should_fail__str_init_move_null_str(
	void
	)
{
	str_t str;
	str_init(&str);
	str_init_move(NULL, &str);
}


void assert_used
test_should_fail__str_init_move_null(
	void
	)
{
	str_init_move(NULL, NULL);
}


void assert_used
test_should_fail__str_clear_null(
	void
	)
{
	str_clear(NULL);
}


void assert_used
test_should_fail__str_is_empty_null(
	void
	)
{
	str_is_empty(NULL);
}


void assert_used
test_should_fail__str_set_copy_cstr_null_str(
	void
	)
{
	str_set_copy_cstr(NULL, "test");
}


void assert_used
test_should_fail__str_set_copy_cstr_null_cstr(
	void
	)
{
	str_t str;
	str_init(&str);
	str_set_copy_cstr(&str, NULL);
}


void assert_used
test_should_fail__str_set_copy_cstr_null(
	void
	)
{
	str_set_copy_cstr(NULL, NULL);
}


void assert_used
test_should_fail__str_set_move_cstr_null_str(
	void
	)
{
	str_set_move_cstr(NULL, "test");
}


void assert_used
test_should_fail__str_set_move_cstr_null_cstr(
	void
	)
{
	str_t str;
	str_init(&str);
	str_set_move_cstr(&str, NULL);
}


void assert_used
test_should_fail__str_set_move_cstr_null(
	void
	)
{
	str_set_move_cstr(NULL, NULL);
}


void assert_used
test_should_fail__str_set_copy_null_str(
	void
	)
{
	str_t str;
	str_init(&str);
	str_set_copy(NULL, &str);
}


void assert_used
test_should_fail__str_set_copy_null_other(
	void
	)
{
	str_t str;
	str_init(&str);
	str_set_copy(&str, NULL);
}


void assert_used
test_should_fail__str_set_copy_null(
	void
	)
{
	str_set_copy(NULL, NULL);
}


void assert_used
test_should_fail__str_set_move_null_str(
	void
	)
{
	str_t str;
	str_init(&str);
	str_set_move(NULL, &str);
}


void assert_used
test_should_fail__str_set_move_null_other(
	void
	)
{
	str_t str;
	str_init(&str);
	str_set_move(&str, NULL);
}


void assert_used
test_should_fail__str_set_move_null(
	void
	)
{
	str_set_move(NULL, NULL);
}


void assert_used
test_should_fail__str_resize_null(
	void
	)
{
	str_resize(NULL, 4);
}


void assert_used
test_should_fail__str_cmp_null_str1(
	void
	)
{
	str_t str = { "test", 4 };
	str_cmp(NULL, &str);
}


void assert_used
test_should_fail__str_cmp_null_str2(
	void
	)
{
	str_t str = { "test", 4 };
	str_cmp(&str, NULL);
}


void assert_used
test_should_fail__str_cmp_null(
	void
	)
{
	str_cmp(NULL, NULL);
}


void assert_used
test_should_fail__str_case_cmp_null_str1(
	void
	)
{
	str_t str = { "test", 4 };
	str_case_cmp(NULL, &str);
}


void assert_used
test_should_fail__str_case_cmp_null_str2(
	void
	)
{
	str_t str = { "test", 4 };
	str_case_cmp(&str, NULL);
}


void assert_used
test_should_fail__str_case_cmp_null(
	void
	)
{
	str_case_cmp(NULL, NULL);
}


void assert_used
test_should_fail__str_cmp_cstr_null_str(
	void
	)
{
	str_cmp_cstr(NULL, "test");
}


void assert_used
test_should_fail__str_cmp_cstr_null_cstr(
	void
	)
{
	str_t str = { "test", 4 };
	str_cmp_cstr(&str, NULL);
}


void assert_used
test_should_fail__str_cmp_cstr_null(
	void
	)
{
	str_cmp_cstr(NULL, NULL);
}


void assert_used
test_should_fail__str_case_cmp_cstr_null_str(
	void
	)
{
	str_case_cmp_cstr(NULL, "test");
}


void assert_used
test_should_fail__str_case_cmp_cstr_null_cstr(
	void
	)
{
	str_t str = { "test", 4 };
	str_case_cmp_cstr(&str, NULL);
}


void assert_used
test_should_fail__str_case_cmp_cstr_null(
	void
	)
{
	str_case_cmp_cstr(NULL, NULL);
}
