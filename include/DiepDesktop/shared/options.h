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

#include <DiepDesktop/shared/str.h>


typedef struct options* options_t;


extern options_t
options_init(
	int argc,
	const char* const* argv
	);


extern void
options_free(
	options_t options
	);


extern void
options_set(
	options_t options,
	const char* key,
	str_t value
	);


extern void
options_set_default(
	options_t options,
	const char* key,
	str_t value
	);


extern const str_t
options_get(
	options_t options,
	const char* key
	);


extern bool
options_exists(
	options_t options,
	const char* key
	);
