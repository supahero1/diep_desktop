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

#include <stdatomic.h>

#define atomic_load_acq(value)							\
({														\
	atomic_load_explicit(value, memory_order_acquire);	\
})

#define atomic_store_rel(value, new_value)							\
({																	\
	atomic_store_explicit(value, new_value, memory_order_release);	\
})

#define atomic_exchange_acq_rel(value, old_value, new_value)	\
({																\
	typeof(old_value) old_val = old_value;						\
	atomic_compare_exchange_strong_explicit(					\
		value, &old_val, new_value,								\
		memory_order_acq_rel, memory_order_acquire);			\
	old_val;													\
})
