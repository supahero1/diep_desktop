/*
 *   Copyright 2024-2025 Franciszek Balcerak
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

#include <DiepDesktop/shared/macro.h>

#include <inttypes.h>


__attribute__((noreturn))
extern void
assert_failed(
	const char* msg1,
	const char* type1,
	const char* msg2,
	const char* type2,
	const char* msg3,
	...
	);


__attribute__((noreturn))
extern void
unreachable_assert_failed(
	const char* msg
	);


extern void
location_logger(
	const char* msg,
	...
	);


#define GET_PRINTF_TYPE(x)		\
_Generic((x),					\
	int8_t:			"%" PRId8,	\
	int16_t:		"%" PRId16,	\
	int32_t:		"%" PRId32,	\
	int64_t:		"%" PRId64,	\
	uint8_t:		"%" PRIu8,	\
	uint16_t:		"%" PRIu16,	\
	uint32_t:		"%" PRIu32,	\
	uint64_t:		"%" PRIu64,	\
	float:			"%f",		\
	double:			"%lf",		\
	long double:	"%Lf",		\
	default:		"%p"		\
)

#define hard_assert_eq(a, b) if(!__builtin_expect(a == b, 1)) assert_fail(a, b, "==", "!=")
#define hard_assert_neq(a, b) if(!__builtin_expect(a != b, 1)) assert_fail(a, b, "!=", "==")
#define hard_assert_true(a) hard_assert_eq(a, true)
#define hard_assert_false(a) hard_assert_eq(a, false)
#define hard_assert_null(a) hard_assert_eq(a, NULL)
#define hard_assert_not_null(a) hard_assert_neq(a, NULL)
#define hard_assert_lt(a, b) if(!__builtin_expect(a < b, 1)) assert_fail(a, b, "<", ">=")
#define hard_assert_le(a, b) if(!__builtin_expect(a <= b, 1)) assert_fail(a, b, "<=", ">")
#define hard_assert_gt(a, b) if(!__builtin_expect(a > b, 1)) assert_fail(a, b, ">", "<=")
#define hard_assert_ge(a, b) if(!__builtin_expect(a >= b, 1)) assert_fail(a, b, ">=", "<")
#define hard_assert_unreachable()	\
unreachable_assert_failed("Unreachable assertion failed, at " __FILE__ ":" MACRO_STR(__LINE__) "\n")
#define hard_assert_log(...) location_logger("at " __FILE__ ":" MACRO_STR(__LINE__) __VA_OPT__(":") "\n" __VA_ARGS__)

#define empty_assert_eq(a, b) if(!__builtin_expect(a == b, 1)) __builtin_unreachable()
#define empty_assert_neq(a, b) if(!__builtin_expect(a != b, 1)) __builtin_unreachable()
#define empty_assert_true(a) empty_assert_eq(a, true)
#define empty_assert_false(a) empty_assert_eq(a, false)
#define empty_assert_null(a) empty_assert_eq(a, NULL)
#define empty_assert_not_null(a) empty_assert_neq(a, NULL)
#define empty_assert_lt(a, b) if(!__builtin_expect(a < b, 1)) __builtin_unreachable()
#define empty_assert_le(a, b) if(!__builtin_expect(a <= b, 1)) __builtin_unreachable()
#define empty_assert_gt(a, b) if(!__builtin_expect(a > b, 1)) __builtin_unreachable()
#define empty_assert_ge(a, b) if(!__builtin_expect(a >= b, 1)) __builtin_unreachable()
#define empty_assert_unreachable() __builtin_unreachable()
#define empty_assert_log()

#ifndef NDEBUG
	#define assert_fail(a, b, Op, ROp)					\
	assert_failed(										\
		"Assertion \"" #a " " Op " " #b "\" failed: '",	\
		GET_PRINTF_TYPE(a),								\
		"' " ROp " '",									\
		GET_PRINTF_TYPE(b),								\
		"', at " __FILE__ ":" MACRO_STR(__LINE__) "\n",	\
		a,												\
		b												\
		)

	#define assert_eq(a, b) hard_assert_eq(a, b)
	#define assert_neq(a, b) hard_assert_neq(a, b)
	#define assert_true(a) hard_assert_true(a)
	#define assert_false(a) hard_assert_false(a)
	#define assert_null(a) hard_assert_null(a)
	#define assert_not_null(a) hard_assert_not_null(a)
	#define assert_lt(a, b) hard_assert_lt(a, b)
	#define assert_le(a, b) hard_assert_le(a, b)
	#define assert_gt(a, b) hard_assert_gt(a, b)
	#define assert_ge(a, b) hard_assert_ge(a, b)
	#define assert_unreachable() hard_assert_unreachable()
	#define assert_log(...) hard_assert_log(__VA_ARGS__)
	#define private
#else
	#define assert_fail(a, b, Op, ROp)					\
	assert_failed(										\
		"Assertion \"(anonymous)\" failed: '",			\
		GET_PRINTF_TYPE(a),								\
		"' " ROp " '",									\
		GET_PRINTF_TYPE(b),								\
		"', at " __FILE__ ":" MACRO_STR(__LINE__) "\n",	\
		a,												\
		b												\
		)

	#define assert_eq(a, b) empty_assert_eq(a, b)
	#define assert_neq(a, b) empty_assert_neq(a, b)
	#define assert_true(a) empty_assert_true(a)
	#define assert_false(a) empty_assert_false(a)
	#define assert_null(a) empty_assert_null(a)
	#define assert_not_null(a) empty_assert_not_null(a)
	#define assert_lt(a, b) empty_assert_lt(a, b)
	#define assert_le(a, b) empty_assert_le(a, b)
	#define assert_gt(a, b) empty_assert_gt(a, b)
	#define assert_ge(a, b) empty_assert_ge(a, b)
	#define assert_unreachable() empty_assert_unreachable()
	#define assert_log(...) empty_assert_log()
	#define private static
#endif

#define assert_fallthrough() __attribute__((fallthrough))
#define assert_ctor __attribute__((constructor))
#define assert_dtor __attribute__((destructor))
