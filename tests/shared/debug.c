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

#include <stddef.h>


void
test_should_pass__assert_eq(
	void
	)
{
	hard_assert_eq(0, 0);
	hard_assert_eq(false, false);

	hard_assert_eq(1, 1);
	hard_assert_eq(true, true);

	hard_assert_eq(-1, -1);

	hard_assert_eq(1.0f, 1.0f);
}


void
test_should_fail__assert_eq_int(
	void
	)
{
	hard_assert_eq(0, 1);
}


void
test_should_fail__assert_eq_bool(
	void
	)
{
	hard_assert_eq(false, true);
}


void
test_should_fail__assert_eq_negative(
	void
	)
{
	hard_assert_eq(1, -1);
}


void
test_should_fail__assert_eq_float(
	void
	)
{
	hard_assert_eq(1.0f, 2.0f);
}


void
test_should_pass__assert_true(
	void
	)
{
	hard_assert_true(1);
	hard_assert_true(-1);
	hard_assert_true(true);
	hard_assert_true(1.0f);
}


void
test_should_fail__assert_true_int(
	void
	)
{
	hard_assert_true(0);
}


void
test_should_fail__assert_true_false(
	void
	)
{
	hard_assert_true(false);
}


void
test_should_fail__assert_true_float(
	void
	)
{
	hard_assert_true(0.0f);
}


void
test_should_pass__assert_false(
	void
	)
{
	hard_assert_false(0);
	hard_assert_false(false);
	hard_assert_false(0.0f);
}


void
test_should_fail__assert_false_int(
	void
	)
{
	hard_assert_false(1);
}


void
test_should_fail__assert_false_true(
	void
	)
{
	hard_assert_false(true);
}


void
test_should_fail__assert_false_negative(
	void
	)
{
	hard_assert_false(-1);
}


void
test_should_fail__assert_false_float(
	void
	)
{
	hard_assert_false(1.0f);
}


void
test_should_pass__assert_null(
	void
	)
{
	hard_assert_null(NULL);
}


void
test_should_fail__assert_null(
	void
	)
{
	hard_assert_null((void *) 1);
}


void
test_should_pass__assert_not_null(
	void
	)
{
	/* Expect no casting warnings */
	hard_assert_not_null((void *) 1);
	hard_assert_not_null((const void*) 1);
	hard_assert_not_null((volatile void*) -1);
	hard_assert_not_null((const void* const*) 1);
	hard_assert_not_null((const volatile void*) 1);
	hard_assert_not_null((const void* volatile*) 1);
	hard_assert_not_null((volatile void* const*) -1);
	hard_assert_not_null((volatile void* volatile*) 1);
	hard_assert_not_null((const volatile void* const*) 1);
	hard_assert_not_null((const volatile void* volatile*) -1);
	hard_assert_not_null((const volatile void* const volatile*) 1);
}


void
test_should_fail__assert_not_null(
	void
	)
{
	hard_assert_not_null(NULL);
}


void
test_should_pass__assert_ptr(
	void
	)
{
	hard_assert_ptr((void *) 1, 0);
	hard_assert_ptr((void *) 1, 1);

	hard_assert_ptr(NULL, 0);
}


void
test_should_fail__assert_ptr_null_with_non_zero_size(
	void
	)
{
	hard_assert_ptr(NULL, 1);
}


void
test_should_pass__assert_lt(
	void
	)
{
	hard_assert_lt(0, 1);
	hard_assert_lt(-1, 0);
	hard_assert_lt(-2, -1);

	hard_assert_lt((void*) 0, (void*) 1);

	hard_assert_lt(1.0f, 2.0f);
}


void
test_should_fail__assert_lt_different_int(
	void
	)
{
	hard_assert_lt(1, 0);
}


void
test_should_fail__assert_lt_same_int(
	void
	)
{
	hard_assert_lt(0, 0);
}


void
test_should_fail__assert_lt_different_negative_int(
	void
	)
{
	hard_assert_lt(-1, -2);
}


void
test_should_fail__assert_lt_same_negative_int(
	void
	)
{
	hard_assert_lt(-1, -1);
}


void
test_should_fail__assert_lt_different_ptr(
	void
	)
{
	hard_assert_lt((void*) 1, (void*) 0);
}


void
test_should_fail__assert_lt_same_ptr(
	void
	)
{
	hard_assert_lt((void*) 0, (void*) 0);
}


void
test_should_fail__assert_lt_different_float(
	void
	)
{
	hard_assert_lt(2.0f, 1.0f);
}


void
test_should_fail__assert_lt_same_float(
	void
	)
{
	hard_assert_lt(1.0f, 1.0f);
}


void
test_should_pass__assert_le(
	void
	)
{
	hard_assert_le(0, 1);
	hard_assert_le(-1, 0);
	hard_assert_le(-2, -1);

	hard_assert_le(1, 1);
	hard_assert_le(0, 0);
	hard_assert_le(-1, -1);

	hard_assert_le((void*) 0, (void*) 1);
	hard_assert_le((void*) 0, (void*) 0);

	hard_assert_le(1.0f, 2.0f);
	hard_assert_le(1.0f, 1.0f);
}


void
test_should_fail__assert_le_different_int(
	void
	)
{
	hard_assert_le(1, 0);
}


void
test_should_fail__assert_le_different_negative_int(
	void
	)
{
	hard_assert_le(-1, -2);
}


void
test_should_fail__assert_le_different_ptr(
	void
	)
{
	hard_assert_le((void*) 1, (void*) 0);
}


void
test_should_fail__assert_le_different_float(
	void
	)
{
	hard_assert_le(2.0f, 1.0f);
}


void
test_should_pass__assert_gt(
	void
	)
{
	hard_assert_gt(1, 0);
	hard_assert_gt(0, -1);
	hard_assert_gt(-1, -2);

	hard_assert_gt((void*) 1, (void*) 0);

	hard_assert_gt(2.0f, 1.0f);
}


void
test_should_fail__assert_gt_different_int(
	void
	)
{
	hard_assert_gt(0, 1);
}


void
test_should_fail__assert_gt_same_int(
	void
	)
{
	hard_assert_gt(0, 0);
}


void
test_should_fail__assert_gt_different_negative_int(
	void
	)
{
	hard_assert_gt(-2, -1);
}


void
test_should_fail__assert_gt_same_negative_int(
	void
	)
{
	hard_assert_gt(-1, -1);
}


void
test_should_fail__assert_gt_different_ptr(
	void
	)
{
	hard_assert_gt((void*) 0, (void*) 1);
}


void
test_should_fail__assert_gt_same_ptr(
	void
	)
{
	hard_assert_gt((void*) 0, (void*) 0);
}


void
test_should_fail__assert_gt_different_float(
	void
	)
{
	hard_assert_gt(1.0f, 2.0f);
}


void
test_should_fail__assert_gt_same_float(
	void
	)
{
	hard_assert_gt(1.0f, 1.0f);
}


void
test_should_pass__assert_ge(
	void
	)
{
	hard_assert_ge(1, 0);
	hard_assert_ge(0, -1);
	hard_assert_ge(-1, -2);

	hard_assert_ge(1, 1);
	hard_assert_ge(0, 0);
	hard_assert_ge(-1, -1);

	hard_assert_ge((void*) 1, (void*) 0);
	hard_assert_ge((void*) 0, (void*) 0);

	hard_assert_ge(2.0f, 1.0f);
	hard_assert_ge(1.0f, 1.0f);
}


void
test_should_fail__assert_ge_different_int(
	void
	)
{
	hard_assert_ge(0, 1);
}


void
test_should_fail__assert_ge_different_negative_int(
	void
	)
{
	hard_assert_ge(-2, -1);
}


void
test_should_fail__assert_ge_different_ptr(
	void
	)
{
	hard_assert_ge((void*) 0, (void*) 1);
}


void
test_should_fail__assert_ge_different_float(
	void
	)
{
	hard_assert_ge(1.0f, 2.0f);
}


void
test_should_pass__assert_unreachable(
	void
	)
{
	if(0)
	{
		hard_assert_unreachable();
	}
}


void
test_should_fail__assert_unreachable(
	void
	)
{
	hard_assert_unreachable();
}


void
test_should_pass__assert_log(
	void
	)
{
	hard_assert_log("test");
}
