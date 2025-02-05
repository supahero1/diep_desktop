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

#include <DiepDesktop/shared/alloc.h>

#ifndef _inline_
	#define _inline_ __attribute__((always_inline)) inline
#endif


_inline_ _pure_func_ _opaque_ alloc_handle_t*
alloc_get_handle(
	alloc_t size
	)
{
	return alloc_get_handle_s(alloc_get_global_state(), size);
}


_inline_ void
alloc_handle_lock_s(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	alloc_handle_lock_h(alloc_get_handle_s(state, size));
}


_inline_ void
alloc_handle_lock(
	alloc_t size
	)
{
	alloc_handle_lock_h(alloc_get_handle(size));
}


_inline_ void
alloc_handle_unlock_s(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	alloc_handle_unlock_h(alloc_get_handle_s(state, size));
}


_inline_ void
alloc_handle_unlock(
	alloc_t size
	)
{
	alloc_handle_unlock_h(alloc_get_handle(size));
}


_inline_ void
alloc_handle_set_flags_s(
	_in_ alloc_state* state,
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_set_flags_h(alloc_get_handle_s(state, size), flags);
}


_inline_ void
alloc_handle_set_flags_us(
	_in_ alloc_state* state,
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_set_flags_uh(alloc_get_handle_s(state, size), flags);
}


_inline_ void
alloc_handle_set_flags(
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_set_flags_h(alloc_get_handle(size), flags);
}


_inline_ void
alloc_handle_set_flags_u(
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_set_flags_uh(alloc_get_handle(size), flags);
}


_inline_ void
alloc_handle_add_flags_s(
	_in_ alloc_state* state,
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_add_flags_h(alloc_get_handle_s(state, size), flags);
}


_inline_ void
alloc_handle_add_flags_us(
	_in_ alloc_state* state,
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_add_flags_uh(alloc_get_handle_s(state, size), flags);
}


_inline_ void
alloc_handle_add_flags(
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_add_flags_h(alloc_get_handle(size), flags);
}


_inline_ void
alloc_handle_add_flags_u(
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_add_flags_uh(alloc_get_handle(size), flags);
}


_inline_ void
alloc_handle_del_flags_s(
	_in_ alloc_state* state,
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_del_flags_h(alloc_get_handle_s(state, size), flags);
}


_inline_ void
alloc_handle_del_flags_us(
	_in_ alloc_state* state,
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_del_flags_uh(alloc_get_handle_s(state, size), flags);
}


_inline_ void
alloc_handle_del_flags(
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_del_flags_h(alloc_get_handle(size), flags);
}


_inline_ void
alloc_handle_del_flags_u(
	alloc_t size,
	alloc_handle_flag_t flags
	)
{
	alloc_handle_del_flags_uh(alloc_get_handle(size), flags);
}


_inline_ alloc_handle_flag_t
alloc_handle_get_flags_s(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	return alloc_handle_get_flags_h(alloc_get_handle_s(state, size));
}


_inline_ alloc_handle_flag_t
alloc_handle_get_flags_us(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	return alloc_handle_get_flags_uh(alloc_get_handle_s(state, size));
}


_inline_ alloc_handle_flag_t
alloc_handle_get_flags(
	alloc_t size
	)
{
	return alloc_handle_get_flags_h(alloc_get_handle(size));
}


_inline_ alloc_handle_flag_t
alloc_handle_get_flags_u(
	alloc_t size
	)
{
	return alloc_handle_get_flags_uh(alloc_get_handle(size));
}


_inline_ void*
alloc_alloc_s(
	_in_ alloc_state* state,
	alloc_t size,
	int zero
	)
{
	return alloc_alloc_h(alloc_get_handle_s(state, size), size, zero);
}


_inline_ void*
alloc_alloc_us(
	_in_ alloc_state* state,
	alloc_t size,
	int zero
	)
{
	return alloc_alloc_uh(alloc_get_handle_s(state, size), size, zero);
}


_inline_ void*
alloc_alloc(
	alloc_t size,
	int zero
	)
{
	return alloc_alloc_h(alloc_get_handle(size), size, zero);
}


_inline_ void*
alloc_alloc_u(
	alloc_t size,
	int zero
	)
{
	return alloc_alloc_uh(alloc_get_handle(size), size, zero);
}


_inline_ void
alloc_free_s(
	_in_ alloc_state* state,
	alloc_t size,
	_in_ void* ptr
	)
{
	alloc_free_h(alloc_get_handle_s(state, size), ptr, size);
}


_inline_ void
alloc_free_us(
	_in_ alloc_state* state,
	alloc_t size,
	_in_ void* ptr
	)
{
	alloc_free_uh(alloc_get_handle_s(state, size), ptr, size);
}


_inline_ void
alloc_free(
	alloc_t size,
	_in_ void* ptr
	)
{
	alloc_free_h(alloc_get_handle(size), ptr, size);
}


_inline_ void
alloc_free_u(
	alloc_t size,
	_in_ void* ptr
	)
{
	alloc_free_uh(alloc_get_handle(size), ptr, size);
}


_inline_ void*
alloc_realloc_s(
	_in_ alloc_state* old_state,
	alloc_t old_size,
	_in_ void* ptr,
	_in_ alloc_state* new_state,
	alloc_t new_size,
	int zero
	)
{
	return allow_realloc_h(alloc_get_handle_s(old_state, old_size), ptr, old_size,
		alloc_get_handle_s(new_state, new_size), new_size, zero);
}


_inline_ void*
alloc_realloc_us(
	_in_ alloc_state* old_state,
	alloc_t old_size,
	_in_ void* ptr,
	_in_ alloc_state* new_state,
	alloc_t new_size,
	int zero
	)
{
	return allow_realloc_uh(alloc_get_handle_s(old_state, old_size), ptr, old_size,
		alloc_get_handle_s(new_state, new_size), new_size, zero);
}


_inline_ void*
alloc_realloc(
	alloc_t old_size,
	_in_ void* ptr,
	alloc_t new_size,
	int zero
	)
{
	return allow_realloc_h(alloc_get_handle(old_size), ptr, old_size,
		alloc_get_handle(new_size), new_size, zero);
}


_inline_ void*
alloc_realloc_u(
	alloc_t old_size,
	_in_ void* ptr,
	alloc_t new_size,
	int zero
	)
{
	return allow_realloc_uh(alloc_get_handle(old_size), ptr, old_size,
		alloc_get_handle(new_size), new_size, zero);
}


_inline_ void*
alloc_malloc_s(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	return alloc_alloc_s(state, size, 0);
}


_inline_ void*
alloc_malloc_us(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	return alloc_alloc_us(state, size, 0);
}


_inline_ void*
alloc_malloc(
	alloc_t size
	)
{
	return alloc_alloc(size, 0);
}


_inline_ void*
alloc_malloc_u(
	alloc_t size
	)
{
	return alloc_alloc_u(size, 0);
}


_inline_ void*
alloc_calloc_s(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	return alloc_alloc_s(state, size, 1);
}


_inline_ void*
alloc_calloc_us(
	_in_ alloc_state* state,
	alloc_t size
	)
{
	return alloc_alloc_us(state, size, 1);
}


_inline_ void*
alloc_calloc(
	alloc_t size
	)
{
	return alloc_alloc(size, 1);
}


_inline_ void*
alloc_calloc_u(
	alloc_t size
	)
{
	return alloc_alloc_u(size, 1);
}


_inline_ void*
alloc_remalloc_s(
	_in_ alloc_state* old_state,
	alloc_t old_size,
	_in_ void* ptr,
	_in_ alloc_state* new_state,
	alloc_t new_size
	)
{
	return alloc_realloc_s(old_state, old_size, ptr, new_state, new_size, 0);
}


_inline_ void*
alloc_remalloc_us(
	_in_ alloc_state* old_state,
	alloc_t old_size,
	_in_ void* ptr,
	_in_ alloc_state* new_state,
	alloc_t new_size
	)
{
	return alloc_realloc_us(old_state, old_size, ptr, new_state, new_size, 0);
}


_inline_ void*
alloc_remalloc(
	alloc_t old_size,
	_in_ void* ptr,
	alloc_t new_size
	)
{
	return alloc_realloc(old_size, ptr, new_size, 0);
}


_inline_ void*
alloc_remalloc_u(
	alloc_t old_size,
	_in_ void* ptr,
	alloc_t new_size
	)
{
	return alloc_realloc_u(old_size, ptr, new_size, 0);
}


_inline_ void*
alloc_recalloc_s(
	_in_ alloc_state* old_state,
	alloc_t old_size,
	_in_ void* ptr,
	_in_ alloc_state* new_state,
	alloc_t new_size
	)
{
	return alloc_realloc_s(old_state, old_size, ptr, new_state, new_size, 1);
}


_inline_ void*
alloc_recalloc_us(
	_in_ alloc_state* old_state,
	alloc_t old_size,
	_in_ void* ptr,
	_in_ alloc_state* new_state,
	alloc_t new_size
	)
{
	return alloc_realloc_us(old_state, old_size, ptr, new_state, new_size, 1);
}


_inline_ void*
alloc_recalloc(
	alloc_t old_size,
	_in_ void* ptr,
	alloc_t new_size
	)
{
	return alloc_realloc(old_size, ptr, new_size, 1);
}


_inline_ void*
alloc_recalloc_u(
	alloc_t old_size,
	_in_ void* ptr,
	alloc_t new_size
	)
{
	return alloc_realloc_u(old_size, ptr, new_size, 1);
}
