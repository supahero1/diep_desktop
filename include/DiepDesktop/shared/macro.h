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

#define MACRO_POWER_OF_2(bit) (1U << (bit))

#define MACRO_GET_BITS(num) __builtin_choose_expr((num) == 1, 0, 32 - __builtin_clz((num) - 1))

#define MACRO_IS_POWER_OF_2(x)	\
({								\
	__typeof__ (x) _x = (x);	\
	(_x & (_x - 1)) == 0;		\
})

#define MACRO_ALIGN_UP(num, mask)						\
({														\
	__typeof__ (mask) _mask = (mask);					\
	__typeof__ (num) result = (__typeof__ (num))(		\
		((__typeof__ (mask)) num + _mask) & ~_mask);	\
	result;												\
})

#define MACRO_ALIGN_UP_CONST(num, mask)	\
((__typeof__ (num)) (((__typeof__ (mask)) num + (mask)) & ~(mask)))

#define MACRO_ALIGN_DOWN(num, mask)					\
({													\
	__typeof__ (mask) _mask = (mask);				\
	__typeof__ (num) result = (__typeof__ (num))(	\
		((__typeof__ (mask)) num) & ~_mask);		\
	result;											\
})

#define MACRO_ALIGN_DOWN_CONST(num, mask)	\
((__typeof__ (num)) (((__typeof__ (mask)) num) & ~(mask)))

#define MACRO_STR2(x) #x
#define MACRO_STR(x) MACRO_STR2(x)

#define MACRO_ENUM_BITS(name)	\
name##__COUNT,					\
name##__BITS = MACRO_GET_BITS( name##__COUNT )

#define MACRO_ENUM_BITS_EXP(name)	\
name##__COUNT,						\
name##__BITS = MACRO_GET_BITS(MACRO_POWER_OF_2( name##__COUNT ))

#define MACRO_TO_BITS(bytes) ((bytes) << 3)

#define MACRO_TO_BYTES(bits) (((bits) + 7) >> 3)

#define MACRO_ARRAY_LEN(a) (sizeof(a)/sizeof((a)[0]))

#define MACRO_MIN(a, b)			\
({								\
    __typeof__ (a) _a = (a);	\
    __typeof__ (b) _b = (b);	\
    _a > _b ? _b : _a;			\
})

#define MACRO_MAX(a, b)			\
({								\
    __typeof__ (a) _a = (a);	\
    __typeof__ (b) _b = (b);	\
    _a > _b ? _a : _b;			\
})

#define MACRO_CLAMP(a, min, max) MACRO_MIN(MACRO_MAX((a), (min)), (max))
#define MACRO_CLAMP_SYM(a, min_max) MACRO_CLAMP((a), -(min_max), (min_max))

#define MACRO_U32_TO_F32(a)	\
({							\
	union					\
	{						\
		float f32;			\
		uint32_t u32;		\
	}						\
	x =						\
	{						\
		.u32 = a			\
	};						\
							\
	x.f32;					\
})

#define MACRO_F32_TO_U32(a)	\
({							\
	union					\
	{						\
		float f32;			\
		uint32_t u32;		\
	}						\
	x =						\
	{						\
		.f32 = a			\
	};						\
							\
	x.u32;					\
})
