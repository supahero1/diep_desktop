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

#define TEST_PTR ((void*) 0x1)


extern bool test_is_on_valgrind;


void
__attribute__((format(printf, 1, 2)))
test_say(
	const char* format,
	...
	);


void
__attribute__((format(printf, 1, 2)))
test_shout(
	const char* format,
	...
	);
