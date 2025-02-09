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

#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>


typedef struct thread_data_internal
{
	thread_data_t data;
}
thread_data_internal_t;


private void*
thread_fn(
	thread_data_internal_t* internal
	)
{
	thread_data_t data = internal->data;
	alloc_free(sizeof(*internal), internal);

	data.fn(data.data);
	return NULL;
}


void
thread_init(
	thread_t* thread,
	thread_data_t data
	)
{
	assert_not_null(data.fn);

	thread_t id;

	thread_data_internal_t* internal = alloc_malloc(sizeof(*internal));
	assert_not_null(internal);

	internal->data = data;

	int status = pthread_create(&id, NULL,
		(void* (*)(void*)) thread_fn, internal);
	hard_assert_eq(status, 0);

	if(thread)
	{
		*thread = id;
	}
}


void
thread_free(
	thread_t* thread
	)
{
	assert_not_null(thread);

	*thread = -1;
}


void
thread_cancel_on(
	void
	)
{
	int status = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	assert_eq(status, 0);
}


void
thread_cancel_off(
	void
	)
{
	int status = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	assert_eq(status, 0);
}


void
thread_async_on(
	void
	)
{
	int status = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	assert_eq(status, 0);
}


void
thread_async_off(
	void
	)
{
	int status = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	assert_eq(status, 0);
}


void
thread_detach(
	thread_t thread
	)
{
	int status = pthread_detach(thread);
	hard_assert_eq(status, 0);
}


void
thread_join(
	thread_t thread
	)
{
	int status = pthread_join(thread, NULL);
	hard_assert_eq(status, 0);
}


private void
thread_cancel(
	thread_t thread
	)
{
	int status = pthread_cancel(thread);
	hard_assert_eq(status, 0);
}


thread_t
thread_self(
	void
	)
{
	return pthread_self();
}


bool
thread_equal(
	thread_t a,
	thread_t b
	)
{
	return pthread_equal(a, b);
}


void
thread_cancel_sync(
	thread_t thread
	)
{
	if(thread_equal(thread, thread_self()))
	{
		thread_detach(thread);
		thread_exit();
	}
	else
	{
		thread_cancel(thread);
		thread_join(thread);
	}
}


void
thread_cancel_async(
	thread_t thread
	)
{
	thread_detach(thread);
	thread_cancel(thread);
}


void
thread_exit(
	void
	)
{
	pthread_exit(NULL);
}


void
thread_sleep(
	uint64_t ns
	)
{
	struct timespec time;
	time.tv_sec = ns / 1000000000;
	time.tv_nsec = ns % 1000000000;

	struct timespec Rem;

	int status;
	while((status = nanosleep(&time, &Rem)))
	{
		if(errno == EINTR)
		{
			time = Rem;
			continue;
		}

		fprintf(stderr, "nanosleep: %s\n", strerror(errno));
		hard_assert_unreachable();
	}
}


private void
threads_resize(
	threads_t* threads,
	uint32_t count
	)
{
	uint32_t new_used = threads->used + count;
	uint32_t new_size;

	if((new_used < (threads->size >> 2)) || (new_used > threads->size))
	{
		new_size = (new_used << 1) | 1;
	}
	else
	{
		return;
	}

	threads->threads = alloc_remalloc(sizeof(thread_t) * threads->size,
		threads->threads, sizeof(thread_t) * new_size);
	assert_not_null(threads->threads);

	threads->size = new_size;
}


void
threads_init(
	threads_t* threads
	)
{
	threads->threads = NULL;
	threads->used = 0;
	threads->size = 0;
}


void
threads_free(
	threads_t* threads
	)
{
	alloc_free(sizeof(thread_t) * threads->size, threads->threads);
}


void
threads_add(
	threads_t* threads,
	thread_data_t data,
	uint32_t count
	)
{
	threads_resize(threads, count);

	thread_t* thread = threads->threads + threads->used;
	thread_t* thread_end = thread + count;

	for(; thread != thread_end; ++thread)
	{
		thread_init(thread, data);
	}

	threads->used += count;
}


void
threads_cancel_sync(
	threads_t* threads,
	uint32_t count
	)
{
	assert_le(count, threads->used);

	thread_t* thread_start = threads->threads + threads->used - count;

	thread_t* thread = thread_start;
	thread_t* thread_end = thread + count;

	thread_t self = thread_self();
	bool found_self = false;

	for(; thread != thread_end; ++thread)
	{
		if(thread_equal(*thread, self))
		{
			found_self = true;
		}
		else
		{
			thread_cancel(*thread);
		}
	}

	for(thread = thread_start; thread != thread_end; ++thread)
	{
		if(!thread_equal(*thread, self))
		{
			thread_join(*thread);
		}
	}

	threads_resize(threads, -count);

	if(found_self)
	{
		thread_detach(self);
		thread_exit();
	}
}


void
threads_cancel_async(
	threads_t* threads,
	uint32_t count
	)
{
	assert_le(count, threads->used);

	thread_t* thread_start = threads->threads + threads->used - count;

	thread_t* thread = thread_start;
	thread_t* thread_end = thread + count;

	thread_t self = thread_self();
	bool found_self = false;

	for(; thread != thread_end; ++thread)
	{
		if(thread_equal(*thread, self))
		{
			found_self = true;
		}
		else
		{
			thread_detach(*thread);
			thread_cancel(*thread);
		}
	}

	threads_resize(threads, -count);

	if(found_self)
	{
		thread_detach(self);
		thread_cancel(self);
	}
}


void
threads_cancel_all_sync(
	threads_t* threads
	)
{
	threads_cancel_sync(threads, threads->used);
}


void
threads_cancel_all_async(
	threads_t* threads
	)
{
	threads_cancel_async(threads, threads->used);
}


void
thread_pool_fn(
	void* data
	)
{
	thread_pool_t* pool = data;

	while(1)
	{
		thread_pool_work(pool);
	}
}


void
thread_pool_init(
	thread_pool_t* pool
	)
{
	sync_sem_init(&pool->sem, 0);
	sync_mtx_init(&pool->mtx);

	pool->queue = NULL;
	pool->used = 0;
	pool->size = 0;
}


void
thread_pool_free(
	thread_pool_t* pool
	)
{
	alloc_free(sizeof(thread_data_t) * pool->size, pool->queue);

	sync_mtx_free(&pool->mtx);
	sync_sem_free(&pool->sem);
}


void
thread_pool_lock(
	thread_pool_t* pool
	)
{
	sync_mtx_lock(&pool->mtx);
}


void
thread_pool_unlock(
	thread_pool_t* pool
	)
{
	sync_mtx_unlock(&pool->mtx);
}


private void
thread_pool_resize(
	thread_pool_t* pool,
	uint32_t count
	)
{
	uint32_t new_used = pool->used + count;
	uint32_t new_size;

	if((new_used < (pool->size >> 2)) || (new_used > pool->size))
	{
		new_size = (new_used << 1) | 1;
	}
	else
	{
		return;
	}

	pool->queue = alloc_remalloc(sizeof(thread_data_t) * pool->size,
		pool->queue, sizeof(thread_data_t) * new_size);
	assert_not_null(pool->queue);

	pool->size = new_size;
}


private void
thread_pool_add_common(
	thread_pool_t* pool,
	thread_data_t data,
	bool lock
	)
{
	if(lock)
	{
		thread_pool_lock(pool);
	}

	thread_pool_resize(pool, 1);

	pool->queue[pool->used++] = data;

	if(lock)
	{
		thread_pool_unlock(pool);
	}

	sync_sem_post(&pool->sem);
}


void
thread_pool_add_u(
	thread_pool_t* pool,
	thread_data_t data
	)
{
	thread_pool_add_common(pool, data, false);
}


void
thread_pool_add(
	thread_pool_t* pool,
	thread_data_t data
	)
{
	thread_pool_add_common(pool, data, true);
}


private void
thread_pool_try_work_common(
	thread_pool_t* pool,
	bool lock
	)
{
	if(lock)
	{
		thread_pool_lock(pool);
	}

	if(!pool->used)
	{
		if(lock)
		{
			thread_pool_unlock(pool);
		}

		return;
	}

	thread_data_t data = *pool->queue;

	thread_pool_resize(pool, -1);

	if(pool->used)
	{
		(void) memmove(pool->queue, pool->queue + 1,
			sizeof(*pool->queue) * pool->used);
	}

	if(lock)
	{
		thread_pool_unlock(pool);
	}

	data.fn(data.data);
}


void
thread_pool_try_work_u(
	thread_pool_t* pool
	)
{
	thread_pool_try_work_common(pool, false);
}


void
thread_pool_try_work(
	thread_pool_t* pool
	)
{
	thread_pool_try_work_common(pool, true);
}


void
thread_pool_work_u(
	thread_pool_t* pool
	)
{
	sync_sem_wait(&pool->sem);

	thread_pool_try_work_u(pool);
}


void
thread_pool_work(
	thread_pool_t* pool
	)
{
	thread_async_off();
		sync_sem_wait(&pool->sem);

		thread_cancel_off();
			thread_pool_try_work(pool);
		thread_cancel_on();
	thread_async_on();
}
