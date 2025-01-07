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

#include <DiepDesktop/shared/sync.h>
#include <DiepDesktop/shared/debug.h>

#include <errno.h>


void
sync_mtx_init(
	sync_mtx_t* mtx
	)
{
	int status = pthread_mutex_init(mtx, NULL);
	hard_assert_eq(status, 0);
}


void
sync_mtx_free(
	sync_mtx_t* mtx
	)
{
	int status = pthread_mutex_destroy(mtx);
	hard_assert_eq(status, 0);
}


void
sync_mtx_lock(
	sync_mtx_t* mtx
	)
{
	int status = pthread_mutex_lock(mtx);
	assert_eq(status, 0);
}


void
sync_mtx_unlock(
	sync_mtx_t* mtx
	)
{
	int status = pthread_mutex_unlock(mtx);
	assert_eq(status, 0);
}


void
sync_cond_init(
	sync_cond_t* cond
	)
{
	int status = pthread_cond_init(cond, NULL);
	hard_assert_eq(status, 0);
}


void
sync_cond_free(
	sync_cond_t* cond
	)
{
	int status = pthread_cond_destroy(cond);
	hard_assert_eq(status, 0);
}


void
sync_cond_wait(
	sync_cond_t* cond,
	sync_mtx_t* mtx
	)
{
	int status = pthread_cond_wait(cond, mtx);
	assert_eq(status, 0);
}


void
sync_cond_wake(
	sync_cond_t* cond
	)
{
	int status = pthread_cond_signal(cond);
	assert_eq(status, 0);
}


void
sync_sem_init(
	sync_sem_t* sem,
	uint32_t value
	)
{
	int status = sem_init(sem, 0, value);
	hard_assert_eq(status, 0);
}


void
sync_sem_free(
	sync_sem_t* sem
	)
{
	int status = sem_destroy(sem);
	hard_assert_eq(status, 0);
}


void
sync_sem_wait(
	sync_sem_t* sem
	)
{
	int status;
	while((status = sem_wait(sem)))
	{
		if(errno == EINTR)
		{
			continue;
		}

		hard_assert_unreachable();
	}
}


void
sync_sem_timed_wait(
	sync_sem_t* sem,
	uint64_t ns
	)
{
	struct timespec time;
	time.tv_sec = ns / 1000000000;
	time.tv_nsec = ns % 1000000000;

	int status;
	while((status = sem_timedwait(sem, &time)))
	{
		if(errno == EINTR)
		{
			continue;
		}

		if(errno == ETIMEDOUT)
		{
			break;
		}

		hard_assert_unreachable();
	}
}


void
sync_sem_post(
	sync_sem_t* sem
	)
{
	int status = sem_post(sem);
	assert_eq(status, 0);
}
