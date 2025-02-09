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


void assert_used
test_should_pass__time_time_conversion(
	void
	)
{
	assert_eq(time_sec_to_ns(0), 0);
	assert_eq(time_sec_to_us(0), 0);
	assert_eq(time_sec_to_ms(0), 0);

	assert_eq(time_ms_to_ns(0), 0);
	assert_eq(time_ms_to_us(0), 0);
	assert_eq(time_ms_to_sec(0), 0);

	assert_eq(time_us_to_ns(0), 0);
	assert_eq(time_us_to_sec(0), 0);
	assert_eq(time_us_to_ms(0), 0);

	assert_eq(time_ns_to_sec(0), 0);
	assert_eq(time_ns_to_ms(0), 0);
	assert_eq(time_ns_to_us(0), 0);

	assert_eq(time_sec_to_ns(1), 1000000000);
	assert_eq(time_sec_to_us(1), 1000000);
	assert_eq(time_sec_to_ms(1), 1000);

	assert_eq(time_ms_to_ns(1), 1000000);
	assert_eq(time_ms_to_us(1), 1000);
	assert_eq(time_ms_to_sec(999), 0);
	assert_eq(time_ms_to_sec(1999), 1);

	assert_eq(time_us_to_ns(1), 1000);
	assert_eq(time_us_to_sec(999999), 0);
	assert_eq(time_us_to_sec(1999999), 1);
	assert_eq(time_us_to_ms(999), 0);
	assert_eq(time_us_to_ms(1999), 1);

	assert_eq(time_ns_to_sec(999999999), 0);
	assert_eq(time_ns_to_sec(1999999999), 1);
	assert_eq(time_ns_to_ms(999999), 0);
	assert_eq(time_ns_to_ms(1999999), 1);
	assert_eq(time_ns_to_us(999), 0);
	assert_eq(time_ns_to_us(1999), 1);
}


void assert_used
test_should_pass__time_timers_init_free(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);
	time_timers_free(&timers);
}


void assert_used
test_should_fail__time_timers_init_null(
	void
	)
{
	time_timers_init(NULL);
}


void assert_used
test_should_fail__time_timers_free_null(
	void
	)
{
	time_timers_free(NULL);
}


void assert_used
test_should_fail__time_timers_lock_null(
	void
	)
{
	time_timers_lock(NULL);
}


void assert_used
test_should_fail__time_timers_unlock_null(
	void
	)
{
	time_timers_unlock(NULL);
}


void assert_used
test_should_pass__time_timer_init_free(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);
	time_timer_free(&timer);
}


void assert_used
test_should_fail__time_timer_init_null(
	void
	)
{
	time_timer_init(NULL);
}


void assert_used
test_should_fail__time_timer_free_null(
	void
	)
{
	time_timer_free(NULL);
}


void assert_used
test_should_pass__time_timers_is_timer_expired_u(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	time_timer_t timer;
	time_timer_init(&timer);

	assert_true(time_timers_is_timer_expired_u(&timers, &timer));

	time_timers_free(&timers);
}


void assert_used
test_should_fail__time_timers_is_timer_expired_u_timers_null(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_is_timer_expired_u(NULL, &timer);
}


void assert_used
test_should_fail__time_timers_is_timer_expired_u_timer_null(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	time_timers_is_timer_expired_u(&timers, NULL);
}


void assert_used
test_should_fail__time_timers_is_timer_expired_u_null(
	void
	)
{
	time_timers_is_timer_expired_u(NULL, NULL);
}


void assert_used
test_should_pass__time_timers_is_timer_expired(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	time_timer_t timer;
	time_timer_init(&timer);

	assert_true(time_timers_is_timer_expired(&timers, &timer));

	time_timers_free(&timers);
}


void assert_used
test_should_fail__time_timers_is_timer_expired_timers_null(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_is_timer_expired(NULL, &timer);
}


void assert_used
test_should_fail__time_timers_is_timer_expired_timer_null(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	time_timers_is_timer_expired(&timers, NULL);
}


void assert_used
test_should_fail__time_timers_is_timer_expired_null(
	void
	)
{
	time_timers_is_timer_expired(NULL, NULL);
}


