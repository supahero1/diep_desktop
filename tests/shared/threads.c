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

#include <DiepDesktop/shared/time.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/shared/alloc_ext.h>


void assert_used
test_should_fail__thread_init_null_fn(
	void
	)
{
	thread_t thread;
	thread_data_t data = {0};
	thread_init(&thread, data);
}


static void
dummy_thread_fn(
	void* data
	)
{
	(void) data;
}


void assert_used
test_should_pass__thread_init_free(
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
test_should_fail__thread_free_null(
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
test_should_timeout__thread_cancel_off(
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
test_should_pass__thread_cancel_on(
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
test_should_timeout__thread_cancel_off_self(
	void
	)
{
	thread_cancel_off();
	thread_cancel_async(thread_self());
	thread_sleep(time_sec_to_ns(99));
}
