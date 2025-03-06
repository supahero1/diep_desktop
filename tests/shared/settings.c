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

#include <DiepDesktop/tests/base.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/settings.h>

#include <string.h>

#define TEST_FILENAME	\
(test_is_on_valgrind ? "bin/tests/shared/settings.bin.val" : "bin/tests/shared/settings.bin")


void assert_used
test_should_pass__settings_init_free(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	settings_t settings;
	settings_init(&settings, TEST_FILENAME, &timers);
	settings_free(&settings);

	time_timers_free(&timers);
}


static void
settings_onsave_fn(
	bool* save,
	settings_save_event_data_t* data
	)
{
	assert_true(data->success);
	*save = true;
}


static void
settings_onload_fn(
	bool* load,
	settings_load_event_data_t* data
	)
{
	assert_true(data->success);
	*load = true;
}


static void
setting_change_i64_fn(
	int64_t* change,
	setting_change_event_data_t* data
	)
{
	*change = data->new_value.i64;
}


static void
setting_change_f32_fn(
	float* change,
	setting_change_event_data_t* data
	)
{
	*change = data->new_value.f32;
}


void assert_used
test_should_pass__settings_save_load(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	settings_t settings;
	settings_init(&settings, TEST_FILENAME, &timers);

	bool saved = false;
	event_listener_data_t save_listener_data =
	{
		.fn = (event_fn_t) settings_onsave_fn,
		.data = &saved
	};
	event_listener_t* save_listener = event_target_add(&settings.save_target, save_listener_data);

	bool loaded = false;
	event_listener_data_t load_listener_data =
	{
		.fn = (event_fn_t) settings_onload_fn,
		.data = &loaded
	};
	event_listener_t* load_listener = event_target_add(&settings.load_target, load_listener_data);

	event_target_t i64_change_target;
	event_target_init(&i64_change_target);

	int64_t i64_change = 0;
	event_listener_data_t i64_listener_data =
	{
		.fn = (event_fn_t) setting_change_i64_fn,
		.data = &i64_change
	};
	event_listener_t* i64_listener = event_target_add(&i64_change_target, i64_listener_data);

	setting_t* i64_s = settings_add_i64(&settings, "foo", 42, 0, 100, &i64_change_target);
	assert_eq(i64_change, 0);
	assert_false(saved);
	assert_false(loaded);

	(void) settings_add_i64(&settings, "foobar", 2, 1, 3, NULL);
	assert_eq(i64_change, 0);
	assert_false(saved);
	assert_false(loaded);

	assert_eq(setting_get_i64(i64_s), 42);

	event_target_t f32_change_target;
	event_target_init(&f32_change_target);

	float f32_change = 0.0f;
	event_listener_data_t f32_listener_data =
	{
		.fn = (event_fn_t) setting_change_f32_fn,
		.data = &f32_change
	};
	event_listener_t* f32_listener = event_target_add(&f32_change_target, f32_listener_data);

	(void) settings_add_f32(&settings, "barfoo", 2.0f, 1.0f, 3.0f, NULL);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	setting_t* f32_s = settings_add_f32(&settings, "bar", 1.23f, 0.0f, 2.0f, &f32_change_target);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	assert_eq(setting_get_f32(f32_s), 1.23f);

	settings_seal(&settings);

	assert_eq(setting_get_i64(i64_s), 42);
	assert_eq(setting_get_f32(f32_s), 1.23f);

	settings_save(&settings);
	assert_eq(i64_change, 0);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	settings_modify_i64(&settings, i64_s, 200);
	assert_eq(i64_change, 100); i64_change = 0;
	assert_false(saved);
	assert_false(loaded);

	settings_modify_i64(&settings, i64_s, 43);
	assert_eq(i64_change, 43); i64_change = 0;
	assert_false(saved);
	assert_false(loaded);

	settings_modify_f32(&settings, f32_s, 2.5f);
	assert_eq(f32_change, 2.0f); f32_change = 0.0f;
	assert_false(saved);
	assert_false(loaded);

	settings_save(&settings);
	assert_eq(i64_change, 0);
	assert_eq(f32_change, 0.0f);
	assert_true(saved); saved = false;
	assert_false(loaded);

	event_target_del(&settings.load_target, load_listener);
	event_target_del(&settings.save_target, save_listener);

	settings_free(&settings);
	time_timers_free(&timers);

	settings_init(&settings, TEST_FILENAME, NULL);
	assert_eq(i64_change, 0);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	save_listener = event_target_add(&settings.save_target, save_listener_data);
	load_listener = event_target_add(&settings.load_target, load_listener_data);

	i64_s = settings_add_i64(&settings, "foo", 44, 40, 45, &i64_change_target);
	assert_eq(i64_change, 0);
	assert_false(saved);
	assert_false(loaded);

	f32_s = settings_add_f32(&settings, "bar", 1.5f, 1.0f, 3.0f, &f32_change_target);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	settings_seal(&settings);

	settings_load(&settings);
	assert_eq(i64_change, 43); i64_change = 0;
	assert_eq(setting_get_i64(i64_s), 43);
	assert_eq(f32_change, 2.0f); f32_change = 0.0f;
	assert_eq(setting_get_f32(f32_s), 2.0f);
	assert_false(saved);
	assert_true(loaded); loaded = false;

	event_target_del(&f32_change_target, f32_listener);
	event_target_free(&f32_change_target);

	event_target_del(&i64_change_target, i64_listener);
	event_target_free(&i64_change_target);

	event_target_del(&settings.load_target, load_listener);
	event_target_del(&settings.save_target, save_listener);

	settings_free(&settings);
}


void assert_used
test_should_fail__settings_init_null_settings(
	void
	)
{
	settings_init(NULL, TEST_FILENAME, NULL);
}


void assert_used
test_should_fail__settings_init_null_path(
	void
	)
{
	settings_t settings;
	settings_init(&settings, NULL, NULL);
}


void assert_used
test_should_fail__settings_init_null(
	void
	)
{
	settings_init(NULL, NULL, NULL);
}


void assert_used
test_should_fail__settings_free_null(
	void
	)
{
	settings_free(NULL);
}


void assert_used
test_should_fail__settings_save_null(
	void
	)
{
	settings_save(NULL);
}


void assert_used
test_should_fail__settings_load_null(
	void
	)
{
	settings_load(NULL);
}


void assert_used
test_should_fail__settings_add_i64_null_settings(
	void
	)
{
	settings_add_i64(NULL, "0", 0, 0, 0, NULL);
}


void assert_used
test_should_fail__settings_add_i64_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	settings_add_i64(&settings, NULL, 0, 0, 0, NULL);
}


void assert_used
test_should_fail__settings_add_i64_null(
	void
	)
{
	settings_add_i64(NULL, NULL, 0, 0, 0, NULL);
}


void assert_used
test_should_fail__settings_add_i64_invalid_value(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	settings_add_i64(&settings, "0", 1, 0, 0, NULL);
}


void assert_used
test_should_fail__settings_add_i64_invalid_constaint(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	settings_add_i64(&settings, "0", 0, 1, 0, NULL);
}


void assert_used
test_should_fail__settings_add_f32_null_settings(
	void
	)
{
	settings_add_f32(NULL, "0", 0.0f, 0.0f, 0.0f, NULL);
}


void assert_used
test_should_fail__settings_add_f32_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	settings_add_f32(&settings, NULL, 0.0f, 0.0f, 0.0f, NULL);
}


void assert_used
test_should_fail__settings_add_f32_null(
	void
	)
{
	settings_add_f32(NULL, NULL, 0.0f, 0.0f, 0.0f, NULL);
}


void assert_used
test_should_fail__settings_add_f32_invalid_value(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	settings_add_f32(&settings, "0", 1.0f, 0.0f, 0.0f, NULL);
}


void assert_used
test_should_fail__settings_add_f32_invalid_constaint(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	settings_add_f32(&settings, "0", 0.0f, 1.0f, 0.0f, NULL);
}


void assert_used
test_should_fail__settings_add_boolean_null_settings(
	void
	)
{
	settings_add_boolean(NULL, "0", false, NULL);
}


void assert_used
test_should_fail__settings_add_boolean_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	settings_add_boolean(&settings, NULL, false, NULL);
}


void assert_used
test_should_fail__settings_add_boolean_null(
	void
	)
{
	settings_add_boolean(NULL, NULL, false, NULL);
}


void assert_used
test_should_fail__settings_add_str_null_settings(
	void
	)
{
	str_t str;
	str_init(&str);

	settings_add_str(NULL, "0", str, 0, NULL);
}


void assert_used
test_should_fail__settings_add_str_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	str_t str;
	str_init(&str);

	settings_add_str(&settings, NULL, str, 0, NULL);
}


void assert_used
test_should_fail__settings_add_str_null(
	void
	)
{
	str_t str;
	str_init(&str);

	settings_add_str(NULL, NULL, str, 0, NULL);
}


void assert_used
test_should_fail__settings_add_color_null_settings(
	void
	)
{
	color_argb_t color = {0};
	settings_add_color(NULL, "0", color, NULL);
}


void assert_used
test_should_fail__settings_add_color_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	color_argb_t color = {0};
	settings_add_color(&settings, NULL, color, NULL);
}


void assert_used
test_should_fail__settings_add_color_null(
	void
	)
{
	color_argb_t color = {0};
	settings_add_color(NULL, NULL, color, NULL);
}


void assert_used
test_should_fail__settings_modify_i64_null_settings(
	void
	)
{
	settings_modify_i64(NULL, TEST_PTR, 0);
}


void assert_used
test_should_fail__settings_modify_i64_null_name(
	void
	)
{
	settings_modify_i64(TEST_PTR, NULL, 0);
}


void assert_used
test_should_fail__settings_modify_i64_null(
	void
	)
{
	settings_modify_i64(NULL, NULL, 0);
}


void assert_used
test_should_fail__settings_modify_f32_null_settings(
	void
	)
{
	settings_modify_f32(NULL, TEST_PTR, 0.0f);
}


void assert_used
test_should_fail__settings_modify_f32_null_name(
	void
	)
{
	settings_modify_f32(TEST_PTR, NULL, 0.0f);
}


void assert_used
test_should_fail__settings_modify_f32_null(
	void
	)
{
	settings_modify_f32(NULL, NULL, 0.0f);
}


void assert_used
test_should_fail__settings_modify_boolean_null_settings(
	void
	)
{
	settings_modify_boolean(NULL, TEST_PTR, false);
}


void assert_used
test_should_fail__settings_modify_boolean_null_name(
	void
	)
{
	settings_modify_boolean(TEST_PTR, NULL, false);
}


void assert_used
test_should_fail__settings_modify_boolean_null(
	void
	)
{
	settings_modify_boolean(NULL, NULL, false);
}


void assert_used
test_should_fail__settings_modify_str_null_settings(
	void
	)
{
	str_t str;
	str_init(&str);

	settings_modify_str(NULL, TEST_PTR, str);
}


void assert_used
test_should_fail__settings_modify_str_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	str_t str;
	str_init(&str);

	settings_modify_str(&settings, NULL, str);
}


void assert_used
test_should_fail__settings_modify_str_null(
	void
	)
{
	str_t str;
	str_init(&str);

	settings_modify_str(NULL, NULL, str);
}


void assert_used
test_should_fail__settings_modify_color_null_settings(
	void
	)
{
	color_argb_t color = {0};
	settings_modify_color(NULL, TEST_PTR, color);
}


void assert_used
test_should_fail__settings_modify_color_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, TEST_FILENAME, NULL);

	color_argb_t color = {0};
	settings_modify_color(&settings, NULL, color);
}


void assert_used
test_should_fail__settings_modify_color_null(
	void
	)
{
	color_argb_t color = {0};
	settings_modify_color(NULL, NULL, color);
}


void assert_used
test_should_fail__setting_get_i64_null(
	void
	)
{
	setting_get_i64(NULL);
}


void assert_used
test_should_fail__setting_get_f32_null(
	void
	)
{
	setting_get_f32(NULL);
}


void assert_used
test_should_fail__setting_get_boolean_null(
	void
	)
{
	setting_get_boolean(NULL);
}


void assert_used
test_should_fail__setting_get_str_null(
	void
	)
{
	setting_get_str(NULL);
}


void assert_used
test_should_fail__setting_get_color_null(
	void
	)
{
	setting_get_color(NULL);
}
