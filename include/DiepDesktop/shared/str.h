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

#pragma once

#include <stdint.h>


typedef struct str
{
	uint8_t* str;
	uint64_t len;
}
str_t;


extern void
str_init(
	str_t* str
	);


extern void
str_init_cstr_copy(
	str_t* str,
	const uint8_t* cstr
	);


extern void
str_init_cstr_move(
	str_t* str,
	uint8_t* cstr
	);


extern void
str_init_len_copy(
	str_t* str,
	const uint8_t* cstr,
	uint64_t len
	);


extern void
str_init_len_move(
	str_t* str,
	uint8_t* cstr,
	uint64_t len
	);


extern void
str_init_copy(
	str_t* str,
	const str_t* other
	);


extern void
str_init_move(
	str_t* str,
	str_t* other
	);


extern void
str_free(
	str_t* str
	);


extern void
str_clear(
	str_t* str
	);


extern void
str_set_cstr_copy(
	str_t* str,
	const uint8_t* cstr
	);


extern void
str_set_cstr_move(
	str_t* str,
	uint8_t* cstr
	);


extern void
str_set_len_copy(
	str_t* str,
	const uint8_t* cstr,
	uint64_t len
	);


extern void
str_set_len_move(
	str_t* str,
	uint8_t* cstr,
	uint64_t len
	);


extern void
str_copy(
	str_t* str,
	const str_t* other
	);


extern void
str_move(
	str_t* str,
	str_t* other
	);


extern bool
str_cmp(
	const str_t* str1,
	const str_t* str2
	);


extern bool
str_case_cmp(
	const str_t* str1,
	const str_t* str2
	);
