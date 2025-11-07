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

#include <shared/sync.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/threads.h>

#include <stdatomic.h>


void assert_used
test_normal_fail__thread_init_null_fn(
	void
	)
{
	thread_data_t data = {0};
	thread_init(NULL, data);
}


static void
dummy_thread_fn(
	void* data
	)
{
	(void) data;
}


void assert_used
test_normal_pass__thread_init_free(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_join(thread);
	thread_free(&thread);
}


void assert_used
test_normal_fail__thread_free_null(
	void
	)
{
	thread_free(NULL);
}


static void
thread_cancel_off_fn(
	void* data
	)
{
	(void) data;
	thread_cancel_off();
	thread_sleep(time_sec_to_ns(99));
}


void assert_used
test_normal_timeout__thread_cancel_off(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = thread_cancel_off_fn
	};
	thread_init(&thread, data);

	thread_sleep(time_ms_to_ns(10));
	thread_cancel_sync(thread);

	thread_free(&thread);
}


static void
thread_cancel_on_fn(
	void* data
	)
{
	(void) data;
	thread_cancel_on();
	thread_sleep(time_sec_to_ns(99));
}


void assert_used
test_normal_pass__thread_cancel_on(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = thread_cancel_on_fn
	};
	thread_init(&thread, data);

	thread_sleep(time_ms_to_ns(10));
	thread_cancel_sync(thread);

	thread_free(&thread);
}


void assert_used
test_normal_timeout__thread_cancel_off_self(
	void
	)
{
	thread_cancel_off();
	thread_cancel_async(thread_self());
	thread_sleep(time_sec_to_ns(99));
}


void assert_used
test_normal_pass__thread_cancel_on_self(
	void
	)
{
	thread_cancel_on();
	thread_cancel_async(thread_self());
	thread_sleep(time_sec_to_ns(99));
}


void assert_used
test_normal_fail__thread_detach_freed(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_free(&thread);

	thread_detach(thread);
}


void assert_used
test_normal_fail__thread_join_freed(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_free(&thread);

	thread_join(thread);
}


void assert_used
test_normal_fail__thread_cancel_freed(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_free(&thread);

	thread_cancel_sync(thread);
}


static void
sync_thread_fn(
	sync_mtx_t* mtx
	)
{
	sync_mtx_unlock(mtx);
}


void assert_used
test_normal_pass__thread_auto_detach(
	void
	)
{
	sync_mtx_t mtx;
	sync_mtx_init(&mtx);
	sync_mtx_lock(&mtx);

	thread_data_t data =
	{
		.fn = (void*) sync_thread_fn,
		.data = &mtx
	};

	thread_init(NULL, data);

	sync_mtx_lock(&mtx);
	sync_mtx_unlock(&mtx);
	sync_mtx_free(&mtx);
}


typedef struct thread_pool_work_data
{
	_Atomic uint32_t counter;
	uint32_t max;
	sync_mtx_t mtx;
}
thread_pool_work_data_t;


static void
thread_pool_work_fn(
	thread_pool_work_data_t* data
	)
{
	if(atomic_fetch_add_explicit(&data->counter, 1, memory_order_relaxed) + 1 == data->max)
	{
		sync_mtx_unlock(&data->mtx);
	}
}


void assert_used
test_normal_pass__thread_pool_and_threads(
	void
	)
{
	thread_pool_work_data_t data =
	{
		.counter = 0,
		.max = 100
	};
	sync_mtx_init(&data.mtx);
	sync_mtx_lock(&data.mtx);

	thread_pool_t pool;
	thread_pool_init(&pool);

	thread_data_t thread_data =
	{
		.fn = (void*) thread_pool_work_fn,
		.data = &data
	};

	thread_pool_lock(&pool);

	for(uint32_t i = 0; i < data.max; i++)
	{
		thread_pool_add_u(&pool, thread_data);
	}

	thread_pool_unlock(&pool);

	threads_t threads;
	threads_init(&threads);

	thread_data_t threads_data =
	{
		.fn = thread_pool_fn,
		.data = &pool
	};

	threads_add(&threads, threads_data, 16);

	sync_mtx_lock(&data.mtx);

	sync_mtx_unlock(&data.mtx);
	sync_mtx_free(&data.mtx);

	threads_cancel_all_sync(&threads);
	threads_free(&threads);

	thread_pool_free(&pool);

	assert_eq(data.counter, data.max);
}


static void
thread_pool_work_manually_fn(
	uint32_t* counter
	)
{
	(*counter)++;
}


void assert_used
test_normal_pass__thread_pool_try_work(
	void
	)
{
	thread_pool_t pool;
	thread_pool_init(&pool);

	uint32_t counter = 0;

	thread_data_t thread_data =
	{
		.fn = (void*) thread_pool_work_manually_fn,
		.data = &counter
	};

	for(uint32_t i = 0; i < 100; i++)
	{
		thread_pool_add(&pool, thread_data);
	}

	while(thread_pool_try_work(&pool));

	assert_eq(counter, 100);

	thread_pool_add_u(&pool, thread_data);
	bool status = thread_pool_try_work_u(&pool);
	assert_true(status);

	thread_pool_lock(&pool);
	thread_pool_unlock(&pool);

	thread_pool_free(&pool);

	assert_eq(counter, 101);
}


void assert_used
test_normal_fail__threads_init_null(
	void
	)
{
	threads_init(NULL);
}


void assert_used
test_normal_fail__threads_free_null(
	void
	)
{
	threads_free(NULL);
}


void assert_used
test_normal_fail__threads_add_null(
	void
	)
{
	threads_add(NULL, (thread_data_t){0}, 0);
}


void assert_used
test_normal_fail__threads_cancel_sync_null(
	void
	)
{
	threads_cancel_sync(NULL, 0);
}


void assert_used
test_normal_fail__threads_cancel_sync_too_many(
	void
	)
{
	threads_t threads;
	threads_init(&threads);

	threads_cancel_sync(&threads, 1);
}


void assert_used
test_normal_fail__threads_cancel_async_null(
	void
	)
{
	threads_cancel_async(NULL, 0);
}


void assert_used
test_normal_fail__threads_cancel_async_too_many(
	void
	)
{
	threads_t threads;
	threads_init(&threads);

	threads_cancel_async(&threads, 1);
}


void assert_used
test_normal_pass__threads_cancel_zero(
	void
	)
{
	threads_t threads;
	threads_init(&threads);

	threads_cancel_sync(&threads, 0);
	threads_cancel_async(&threads, 0);

	threads_free(&threads);
}





void assert_used
test_normal_fail__thread_pool_init_null(
	void
	)
{
	thread_pool_init(NULL);
}


void assert_used
test_normal_fail__thread_pool_free_null(
	void
	)
{
	thread_pool_free(NULL);
}


void assert_used
test_normal_fail__thread_pool_lock_null(
	void
	)
{
	thread_pool_lock(NULL);
}


void assert_used
test_normal_fail__thread_pool_unlock_null(
	void
	)
{
	thread_pool_unlock(NULL);
}


void assert_used
test_normal_fail__thread_pool_add_null(
	void
	)
{
	thread_pool_add(NULL, (thread_data_t){0});
}


void assert_used
test_normal_fail__thread_pool_add_u_null(
	void
	)
{
	thread_pool_add_u(NULL, (thread_data_t){0});
}


void assert_used
test_normal_fail__thread_pool_add_null_fn(
	void
	)
{
	thread_pool_t pool;
	thread_pool_init(&pool);

	thread_pool_add(&pool, (thread_data_t){0});
}


void assert_used
test_normal_fail__thread_pool_add_u_null_fn(
	void
	)
{
	thread_pool_t pool;
	thread_pool_init(&pool);

	thread_pool_add_u(&pool, (thread_data_t){0});
}


void assert_used
test_normal_fail__thread_pool_try_work_null(
	void
	)
{
	thread_pool_try_work(NULL);
}


void assert_used
test_normal_fail__thread_pool_try_work_u_null(
	void
	)
{
	thread_pool_try_work_u(NULL);
}


void assert_used
test_normal_fail__thread_pool_work_null(
	void
	)
{
	thread_pool_work(NULL);
}


void assert_used
test_normal_fail__thread_pool_work_u_null(
	void
	)
{
	thread_pool_work_u(NULL);
}
