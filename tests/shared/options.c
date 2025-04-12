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

#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/options.h>

#include <stddef.h>


void assert_used
test_normal_pass__options_init_free(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_free(options);
}


void assert_used
test_normal_pass__options_parse(
	void
	)
{
	const char* argv[] =
	{
		"program",
		"--key=value",
		"--foo",
		"--ba",
		"--barr=",
		"123",
		"--",
		"-",
		"",
		"--=1",
		"--=2"
	};

	options_t options = options_init(MACRO_ARRAY_LEN(argv), argv);

	options_set_default(options, "grr", str_init_copy_cstr("grr"));
	options_set_default(options, "ba", str_init_copy_cstr("ba"));

	assert_false(options_exists(options, "program"));

	assert_true(options_exists(options, "key"));
	str_t value = options_get(options, "key");
	assert_true(str_cmp_cstr(value, "value"));

	assert_true(options_exists(options, "foo"));
	value = options_get(options, "foo");
	assert_null(value);

	assert_true(options_exists(options, "ba"));
	value = options_get(options, "ba");
	assert_null(value);

	assert_false(options_exists(options, "bar"));

	value = options_get(options, "barr");
	assert_true(str_cmp_cstr(value, ""));

	value = options_get(options, "grr");
	assert_true(str_cmp_cstr(value, "grr"));

	assert_false(options_exists(options, "123"));

	assert_true(options_exists(options, ""));
	value = options_get(options, "");
	assert_true(str_cmp_cstr(value, "2"));

	assert_false(options_exists(options, "-"));

	options_set(options, "-", str_init_copy_cstr("1"));
	value = options_get(options, "-");
	assert_true(str_cmp_cstr(value, "1"));

	options_free(options);
}


void assert_used
test_normal_fail__options_free_null(
	void
	)
{
	options_free(NULL);
}


void assert_used
test_normal_fail__options_set_null_options(
	void
	)
{
	options_set(NULL, "key", NULL);
}


void assert_used
test_normal_fail__options_set_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_set(options, NULL, NULL);
}


void assert_used
test_normal_fail__options_set_null(
	void
	)
{
	options_set(NULL, NULL, NULL);
}


void assert_used
test_normal_fail__options_set_default_null_options(
	void
	)
{
	options_set_default(NULL, "key", NULL);
}


void assert_used
test_normal_fail__options_set_default_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_set_default(options, NULL, NULL);
}


void assert_used
test_normal_fail__options_set_default_null(
	void
	)
{
	options_set_default(NULL, NULL, NULL);
}


void assert_used
test_normal_fail__options_get_null_options(
	void
	)
{
	options_get(NULL, "key");
}


void assert_used
test_normal_fail__options_get_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_get(options, NULL);
}


void assert_used
test_normal_fail__options_get_null(
	void
	)
{
	options_get(NULL, NULL);
}


void assert_used
test_normal_fail__options_exists_null_options(
	void
	)
{
	options_exists(NULL, "key");
}


void assert_used
test_normal_fail__options_exists_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_exists(options, NULL);
}


void assert_used
test_normal_fail__options_exists_null(
	void
	)
{
	options_exists(NULL, NULL);
}
