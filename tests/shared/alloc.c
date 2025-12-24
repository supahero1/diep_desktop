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

#include <shared/debug.h>
#include <shared/alloc_ext.h>


void assert_used
test_normal_pass__alloc_zero(
	void
	)
{
	void* ptr = alloc_malloc(ptr, 0);
	assert_null(ptr);
}


void assert_used
test_normal_pass__alloc_free_void(
	void
	)
{
	uint32_t* ptr = alloc_malloc(ptr, 16);
	assert_not_null(ptr);

	alloc_free((void*) ptr, 4 * 16);
}


void assert_used
test_priority_pass__alloc_2048_sizes(
	void
	)
{
	uint8_t** ptrs = alloc_malloc(ptrs, 2049);
	assert_not_null(ptrs);

	for(uint64_t i = 1; i <= 2048; ++i)
	{
		uint8_t* ptr = alloc_malloc(ptr, i);
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

	alloc_free(ptrs, 2049);
}


void assert_used
test_normal_pass__alloc_null(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 0);
	assert_null(ptr);

	ptr = alloc_calloc(ptr, 0);
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
	uint8_t* ptr = alloc_malloc(ptr, 1);
	assert_not_null(ptr);
	alloc_free(ptr, 1);

	ptr = alloc_calloc(ptr, 1);
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
	uint8_t* ptr = alloc_malloc(ptr, 1);
	assert_not_null(ptr);

	ptr = alloc_remalloc(ptr, 1, 0);
	assert_null(ptr);

	ptr = alloc_calloc(ptr, 1);
	assert_not_null(ptr);

	ptr = alloc_recalloc(ptr, 1, 0);
	assert_null(ptr);
}


void assert_used
test_normal_pass__alloc_realloc_with_data(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 1);
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
	uint8_t* ptr = alloc_malloc(ptr, 1);
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
	uint8_t* ptr = alloc_malloc(ptr, 1);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_2_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 2);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_4_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 4);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_10_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 10);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_20_to_10(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 20);
	assert_not_null(ptr);

	alloc_free(ptr, 10);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_20_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 20);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_40_to_20(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 40);
	assert_not_null(ptr);

	alloc_free(ptr, 20);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_40_to_10(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 40);
	assert_not_null(ptr);

	alloc_free(ptr, 10);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_40_to_0(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 40);
	assert_not_null(ptr);

	alloc_free(ptr, 0);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_10_to_20(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 10);
	assert_not_null(ptr);

	alloc_free(ptr, 20);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_10_to_40(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 10);
	assert_not_null(ptr);

	alloc_free(ptr, 40);
}


void assert_used
test_normal_fail__alloc_free_invalid_size_20_to_40(
	void
	)
{
	uint8_t* ptr = alloc_malloc(ptr, 20);
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
	uint8_t* ptr = alloc_malloc(ptr, 4);
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

	uint8_t* ptr = alloc_malloc(ptr, 4);
	assert_not_null(ptr);

	volatile uint8_t val1 = *ptr;
	(void) val1;

	alloc_free(ptr, 4);

	ptr = alloc_malloc(ptr, 4);
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

	uint8_t* ptr = alloc_calloc(ptr, 4);
	assert_not_null(ptr);

	volatile uint8_t val1 = *ptr;
	(void) val1;

	alloc_free(ptr, 4);

	ptr = alloc_calloc(ptr, 4);
	assert_not_null(ptr);

	volatile uint8_t val2 = *ptr;
	(void) val2;

	alloc_free(ptr, 4);
}


void assert_used
test_normal_pass__alloc_custom_handle_size_3_align_2(
	void
	)
{
	alloc_handle_t handle = {0};
	alloc_create_handle(
		&(alloc_handle_info_t)
		{
			.alloc_size = 3,
			.block_size = 4096,
			.alignment = 2
		},
		&handle
		);

	uint8_t* ptrs[8];

	for(int i = 0; i < 8; ++i)
	{
		ptrs[i] = alloc_alloc_h(&handle, 3, 0);
		assert_not_null(ptrs[i]);
		ptrs[i][0] = 0xAA ^ i;
		ptrs[i][1] = 0xBB ^ i;
		ptrs[i][2] = 0xCC ^ i;
	}

	for(int i = 0; i < 8; ++i)
	{
		assert_eq(ptrs[i][0], (uint8_t)(0xAA ^ i));
		assert_eq(ptrs[i][1], (uint8_t)(0xBB ^ i));
		assert_eq(ptrs[i][2], (uint8_t)(0xCC ^ i));
	}

	for(int i = 0; i < 8; ++i)
	{
		alloc_free_h(&handle, ptrs[i], 3);
	}

	alloc_free_handle(&handle);
}


void assert_used
test_normal_pass__alloc_custom_handle_size_3_align_4(
	void
	)
{
	alloc_handle_t handle = {0};
	alloc_create_handle(
		&(alloc_handle_info_t)
		{
			.alloc_size = 3,
			.block_size = 4096,
			.alignment = 4
		},
		&handle
		);

	uint8_t* ptrs[8];

	for(int i = 0; i < 8; ++i)
	{
		ptrs[i] = alloc_alloc_h(&handle, 3, 0);
		assert_not_null(ptrs[i]);
		ptrs[i][0] = 0x11 ^ i;
		ptrs[i][1] = 0x22 ^ i;
		ptrs[i][2] = 0x33 ^ i;
	}

	for(int i = 0; i < 8; ++i)
	{
		assert_eq(ptrs[i][0], (uint8_t)(0x11 ^ i));
		assert_eq(ptrs[i][1], (uint8_t)(0x22 ^ i));
		assert_eq(ptrs[i][2], (uint8_t)(0x33 ^ i));
	}

	for(int i = 0; i < 8; ++i)
	{
		alloc_free_h(&handle, ptrs[i], 3);
	}

	alloc_free_handle(&handle);
}


void assert_used
test_normal_pass__alloc_custom_handle_size_3_align_8(
	void
	)
{
	alloc_handle_t handle = {0};
	alloc_create_handle(
		&(alloc_handle_info_t)
		{
			.alloc_size = 3,
			.block_size = 4096,
			.alignment = 8
		},
		&handle
		);

	uint8_t* ptrs[8];

	for(int i = 0; i < 8; ++i)
	{
		ptrs[i] = alloc_alloc_h(&handle, 3, 0);
		assert_not_null(ptrs[i]);
		ptrs[i][0] = 0x44 ^ i;
		ptrs[i][1] = 0x55 ^ i;
		ptrs[i][2] = 0x66 ^ i;
	}

	for(int i = 0; i < 8; ++i)
	{
		assert_eq(ptrs[i][0], (uint8_t)(0x44 ^ i));
		assert_eq(ptrs[i][1], (uint8_t)(0x55 ^ i));
		assert_eq(ptrs[i][2], (uint8_t)(0x66 ^ i));
	}

	for(int i = 0; i < 8; ++i)
	{
		alloc_free_h(&handle, ptrs[i], 3);
	}

	alloc_free_handle(&handle);
}


void assert_used
test_normal_pass__alloc_custom_handle_size_3_align_16(
	void
	)
{
	alloc_handle_t handle = {0};
	alloc_create_handle(
		&(alloc_handle_info_t)
		{
			.alloc_size = 3,
			.block_size = 4096,
			.alignment = 16
		},
		&handle
		);

	uint8_t* ptrs[8];

	for(int i = 0; i < 8; ++i)
	{
		ptrs[i] = alloc_alloc_h(&handle, 3, 0);
		assert_not_null(ptrs[i]);
		ptrs[i][0] = 0x77 ^ i;
		ptrs[i][1] = 0x88 ^ i;
		ptrs[i][2] = 0x99 ^ i;
	}

	for(int i = 0; i < 8; ++i)
	{
		assert_eq(ptrs[i][0], (uint8_t)(0x77 ^ i));
		assert_eq(ptrs[i][1], (uint8_t)(0x88 ^ i));
		assert_eq(ptrs[i][2], (uint8_t)(0x99 ^ i));
	}

	for(int i = 0; i < 8; ++i)
	{
		alloc_free_h(&handle, ptrs[i], 3);
	}

	alloc_free_handle(&handle);
}


void assert_used
test_normal_pass__alloc_custom_handle_size_3_reuse(
	void
	)
{
	alloc_handle_t handle = {0};
	alloc_create_handle(
		&(alloc_handle_info_t)
		{
			.alloc_size = 3,
			.block_size = 4096,
			.alignment = 4
		},
		&handle
		);

	uint8_t* ptr1 = alloc_alloc_h(&handle, 3, 0);
	assert_not_null(ptr1);
	ptr1[0] = 0xDE;
	ptr1[1] = 0xAD;
	ptr1[2] = 0xBE;

	uint8_t* ptr2 = alloc_alloc_h(&handle, 3, 0);
	assert_not_null(ptr2);
	ptr2[0] = 0xCA;
	ptr2[1] = 0xFE;
	ptr2[2] = 0xBA;

	assert_eq(ptr1[0], 0xDE);
	assert_eq(ptr1[1], 0xAD);
	assert_eq(ptr1[2], 0xBE);

	alloc_free_h(&handle, ptr1, 3);

	uint8_t* ptr3 = alloc_alloc_h(&handle, 3, 0);
	assert_not_null(ptr3);
	ptr3[0] = 0x12;
	ptr3[1] = 0x34;
	ptr3[2] = 0x56;

	assert_eq(ptr2[0], 0xCA);
	assert_eq(ptr2[1], 0xFE);
	assert_eq(ptr2[2], 0xBA);

	assert_eq(ptr3[0], 0x12);
	assert_eq(ptr3[1], 0x34);
	assert_eq(ptr3[2], 0x56);

	alloc_free_h(&handle, ptr2, 3);
	alloc_free_h(&handle, ptr3, 3);

	alloc_free_handle(&handle);
}


void assert_used
test_normal_pass__alloc_custom_handle_size_3_calloc(
	void
	)
{
	alloc_handle_t handle = {0};
	alloc_create_handle(
		&(alloc_handle_info_t)
		{
			.alloc_size = 3,
			.block_size = 4096,
			.alignment = 4
		},
		&handle
		);

	uint8_t* ptr = alloc_alloc_h(&handle, 3, 1);
	assert_not_null(ptr);
	assert_eq(ptr[0], 0);
	assert_eq(ptr[1], 0);
	assert_eq(ptr[2], 0);

	alloc_free_h(&handle, ptr, 3);

	alloc_free_handle(&handle);
}


void assert_used
test_normal_pass__alloc_custom_handle_fill_block(
	void
	)
{
	alloc_t page_size = alloc_get_page_size();
	uint32_t count = page_size / 4;

	alloc_handle_t handle = {0};
	alloc_create_handle(
		&(alloc_handle_info_t)
		{
			.alloc_size = 4,
			.block_size = page_size,
			.alignment = 4
		},
		&handle
		);

	uint32_t** ptrs = alloc_malloc(ptrs, count);
	assert_not_null(ptrs);

	for(uint32_t i = 0; i < count; ++i)
	{
		ptrs[i] = alloc_alloc_h(&handle, 4, 0);
		assert_not_null(ptrs[i]);
		*ptrs[i] = 0xDEAD0000 | i;
	}

	for(uint32_t i = 0; i < count; ++i)
	{
		assert_eq(*ptrs[i], 0xDEAD0000 | i);
	}

	uint32_t half = count / 2;
	for(uint32_t i = count; i > half; --i)
	{
		alloc_free_h(&handle, ptrs[i - 1], 4);
		ptrs[i - 1] = NULL;
	}

	for(uint32_t i = 0; i < half; ++i)
	{
		assert_eq(*ptrs[i], 0xDEAD0000 | i);
	}

	for(uint32_t i = half; i < count; ++i)
	{
		ptrs[i] = alloc_alloc_h(&handle, 4, 0);
		assert_not_null(ptrs[i]);
		*ptrs[i] = 0xBEEF0000 | i;
	}

	for(uint32_t i = 0; i < half; ++i)
	{
		assert_eq(*ptrs[i], 0xDEAD0000 | i);
	}

	for(uint32_t i = half; i < count; ++i)
	{
		assert_eq(*ptrs[i], 0xBEEF0000 | i);
	}

	for(uint32_t i = 0; i < count; ++i)
	{
		alloc_free_h(&handle, ptrs[i], 4);
	}

	alloc_free(ptrs, count);

	alloc_free_handle(&handle);
}
