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

/*
 * Test comments:
 *
 * *) Cannot test alloc_free_invalid_size_x_to_y where x or y are {1, 2, 3},
 *   because the results are unreliable (can easily get a segfault).
 *
 * 1) Check for no Valgrind warnings in the output.
 */

#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>


void assert_used
test_normal_pass__alloc_zero(
	void
	)
{
	void* ptr = alloc_malloc(0);
	assert_null(ptr);
}


void assert_used
test_priority_pass__alloc_2048_sizes(
	void
	)
{
	uint8_t** ptrs = alloc_malloc(sizeof(*ptrs) * 2049);
	assert_not_null(ptrs);

	for(uint64_t i = 1; i <= 2048; ++i)
	{
		uint8_t* ptr = alloc_malloc(i);
		assert_not_null(ptr);

		for(uint64_t j = 0; j < i; ++j)
		{
			ptr[j] = (i * j * j) ^ 0x55;
		}

		ptrs[i] = ptr;
	}

	for(uint64_t i = 1; i <= 2048; ++i)
	{
		for(uint64_t j = 0; j < i; ++j)
		{
			assert_eq((i * j * j) ^ 0x55, ptrs[i][j]);
		}
	}

	for(uint64_t i = 1; i <= 2048; ++i)
	{
		alloc_free(ptrs[i], i);
	}

	alloc_free(ptrs, sizeof(*ptrs) * 2049);
}


void assert_used
test_normal_pass__alloc_null(
	void
	)
{
	uint8_t* ptr = alloc_malloc(0);
	assert_null(ptr);

	ptr = alloc_calloc(0);
	assert_null(ptr);

	ptr = alloc_remalloc(NULL, 0, 0);
	assert_null(ptr);

	ptr = alloc_recalloc(NULL, 0, 0);
	assert_null(ptr);

	alloc_free(NULL, 0);
}


void assert_used
test_normal_pass__alloc_one(
	void
	)
{
	uint8_t* ptr = alloc_malloc(1);
	assert_not_null(ptr);
	alloc_free(ptr, 1);

	ptr = alloc_calloc(1);
	assert_not_null(ptr);
	assert_false(*ptr);
	alloc_free(ptr, 1);

	ptr = alloc_remalloc(NULL, 0, 1);
	assert_not_null(ptr);
	alloc_free(ptr, 1);

	ptr = alloc_recalloc(NULL, 0, 1);
	assert_not_null(ptr);
	assert_false(*ptr);
	alloc_free(ptr, 1);
}


void assert_used
test_normal_pass__alloc_realloc_to_zero(
	void
	)
{
	uint8_t* ptr = alloc_malloc(1);
	assert_not_null(ptr);

	ptr = alloc_remalloc(ptr, 1, 0);
	assert_null(ptr);

	ptr = alloc_calloc(1);
	assert_not_null(ptr);

	ptr = alloc_recalloc(ptr, 1, 0);
	assert_null(ptr);
}


void assert_used
test_normal_pass__alloc_realloc_with_data(
	void
	)
{
	uint8_t* ptr = alloc_malloc(1);
	assert_not_null(ptr);
	*ptr = 0x55;

	ptr = alloc_remalloc(ptr, 1, 2);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);
	*(ptr + 1) = 0xAA;

	ptr = alloc_remalloc(ptr, 2, 3);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);
	assert_eq(*(ptr + 1), 0xAA);
	*(ptr + 2) = 0x33;

	ptr = alloc_remalloc(ptr, 3, 2);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);
	assert_eq(*(ptr + 1), 0xAA);

	ptr = alloc_remalloc(ptr, 2, 1);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);

	alloc_free(ptr, 1);
}


void assert_used
test_normal_pass__alloc_recalloc(
	void
	)
{
	uint8_t* ptr = alloc_malloc(1);
	assert_not_null(ptr);
	*ptr = 0x55;

	ptr = alloc_recalloc(ptr, 1, 2);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);
	assert_false(*(ptr + 1));

	ptr = alloc_recalloc(ptr, 2, 3);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);
	assert_false(*(ptr + 1));
	assert_false(*(ptr + 2));

	ptr = alloc_recalloc(ptr, 3, 2);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);
	assert_false(*(ptr + 1));

	ptr = alloc_recalloc(ptr, 2, 1);
	assert_not_null(ptr);
	assert_eq(*ptr, 0x55);

	alloc_free(ptr, 1);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_1_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(1);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_2_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(2);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_4_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(4);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_10_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(10);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_20_to_10(
	void
	)
{
	uint8_t* ptr = alloc_malloc(20);
	assert_not_null(ptr);

	alloc_free(ptr, 10);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_20_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(20);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_40_to_20(
	void
	)
{
	uint8_t* ptr = alloc_malloc(40);
	assert_not_null(ptr);

	alloc_free(ptr, 20);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_40_to_10(
	void
	)
{
	uint8_t* ptr = alloc_malloc(40);
	assert_not_null(ptr);

	alloc_free(ptr, 10);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_40_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(40);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_10_to_20(
	void
	)
{
	uint8_t* ptr = alloc_malloc(10);
	assert_not_null(ptr);

	alloc_free(ptr, 20);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_10_to_40(
	void
	)
{
	uint8_t* ptr = alloc_malloc(10);
	assert_not_null(ptr);

	alloc_free(ptr, 40);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_20_to_40(
	void
	)
{
	uint8_t* ptr = alloc_malloc(20);
	assert_not_null(ptr);

	alloc_free(ptr, 40);
}


void assert_used
test_normal_fail__alloc_free_null_with_size(
	void
	)
{
	alloc_free(NULL, 1);
}


void assert_used
test_normal_fail__alloc_free_unaligned_allocated_ptr(
	void
	)
{
	uint8_t* ptr = alloc_malloc(4);
	assert_not_null(ptr);

	alloc_free(ptr + 1, 4);
}


void assert_used
test_normal_fail__alloc_free_unaligned_random_ptr(
	void
	)
{
	alloc_free((void*) 0x11, 4);
}


void assert_used
test_normal_pass__alloc_reuse_spot_malloc(
	void
	)
{
	/* 1) */

	uint8_t* ptr = alloc_malloc(4);
	assert_not_null(ptr);

	volatile uint8_t val1 = *ptr;
	(void) val1;

	alloc_free(ptr, 4);

	ptr = alloc_malloc(4);
	assert_not_null(ptr);

	volatile uint8_t val2 = *ptr;
	(void) val2;

	alloc_free(ptr, 4);
}


void assert_used
test_normal_pass__alloc_reuse_spot_calloc(
	void
	)
{
	/* 1) */

	uint8_t* ptr = alloc_calloc(4);
	assert_not_null(ptr);

	volatile uint8_t val1 = *ptr;
	(void) val1;

	alloc_free(ptr, 4);

	ptr = alloc_calloc(4);
	assert_not_null(ptr);

	volatile uint8_t val2 = *ptr;
	(void) val2;

	alloc_free(ptr, 4);
}
