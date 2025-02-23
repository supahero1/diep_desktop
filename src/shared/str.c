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
#include <DiepDesktop/shared/alloc_ext.h>

#include <string.h>


void
str_init(
	str_t* str
	)
{
	assert_not_null(str);

	str->str = NULL;
	str->len = 0;
}


void
str_init_cstr_copy(
	str_t* str,
	const uint8_t* cstr
	)
{
	assert_not_null(cstr);

	str_t other =
	{
		.str = (void*) cstr,
		.len = strlen((void*) cstr)
	};
	str_init_copy(str, &other);
}


void
str_init_cstr_move(
	str_t* str,
	uint8_t* cstr
	)
{
	assert_not_null(cstr);

	str_t other =
	{
		.str = cstr,
		.len = strlen((void*) cstr)
	};
	str_init_move(str, &other);
}


void
str_init_len_copy(
	str_t* str,
	const uint8_t* cstr,
	uint64_t len
	)
{
	assert_ptr(cstr, len);

	str_t other =
	{
		.str = (void*) cstr,
		.len = len
	};
	str_init_copy(str, &other);
}


void
str_init_len_move(
	str_t* str,
	uint8_t* cstr,
	uint64_t len
	)
{
	assert_not_null(cstr);

	str_t other =
	{
		.str = cstr,
		.len = len
	};
	str_init_move(str, &other);
}


void
str_init_copy(
	str_t* str,
	const str_t* other
	)
{
	assert_not_null(str);
	assert_not_null(other);
	assert_ptr(other->str, other->len);

	if(!other->str)
	{
		str_init(str);
	}
	else
	{
		str->len = other->len;
		str->str = alloc_malloc(str->len + 1);
		assert_not_null(str->str);

		(void) memcpy(str->str, other->str, str->len + 1);
	}
}


void
str_init_move(
	str_t* str,
	str_t* other
	)
{
	assert_not_null(str);
	assert_not_null(other);
	assert_ptr(other->str, other->len);

	str->len = other->len;
	str->str = other->str;

	str_init(other);
}


void
str_free(
	str_t* str
	)
{
	assert_not_null(str);

	if(str->str)
	{
		alloc_free(str->len + 1, str->str);
	}
	else
	{
		assert_eq(str->len, 0);
	}
}


void
str_clear(
	str_t* str
	)
{
	str_free(str);
	str_init(str);
}


void
str_set_cstr_copy(
	str_t* str,
	const uint8_t* cstr
	)
{
	str_free(str);
	str_init_cstr_copy(str, cstr);
}


void
str_set_cstr_move(
	str_t* str,
	uint8_t* cstr
	)
{
	str_free(str);
	str_init_cstr_move(str, cstr);
}


void
str_set_len_copy(
	str_t* str,
	const uint8_t* cstr,
	uint64_t len
	)
{
	str_free(str);
	str_init_len_copy(str, cstr, len);
}


void
str_set_len_move(
	str_t* str,
	uint8_t* cstr,
	uint64_t len
	)
{
	str_free(str);
	str_init_len_move(str, cstr, len);
}


void
str_set_copy(
	str_t* str,
	const str_t* other
	)
{
	str_free(str);
	str_init_copy(str, other);
}


void
str_set_move(
	str_t* str,
	str_t* other
	)
{
	str_free(str);
	str_init_move(str, other);
}


bool
str_cmp(
	const str_t* str1,
	const str_t* str2
	)
{
	assert_not_null(str1);
	assert_not_null(str2);
	assert_ptr(str1->str, str1->len);
	assert_ptr(str2->str, str2->len);

	if(str1->len != str2->len)
	{
		return false;
	}

	return !strncmp((void*) str1->str, (void*) str2->str, str1->len);
}


bool
str_case_cmp(
	const str_t* str1,
	const str_t* str2
	)
{
	assert_not_null(str1);
	assert_not_null(str2);
	assert_ptr(str1->str, str1->len);
	assert_ptr(str2->str, str2->len);

	if(str1->len != str2->len)
	{
		return false;
	}

	return !strncasecmp((void*) str1->str, (void*) str2->str, str1->len);
}
