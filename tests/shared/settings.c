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

#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/settings.h>

#include <valgrind/valgrind.h>
#define FILENAME (RUNNING_ON_VALGRIND ? "bin/tests/shared/settings.bin.val" : "bin/tests/shared/settings.bin")

#include <string.h>


void assert_used
test_should_pass__settings_init_free(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	settings_t settings;
	settings_init(&settings, FILENAME, &timers);
	settings_free(&settings);

	time_timers_free(&timers);
}


void assert_used
test_should_fail__settings_init_null_settings(
	void
	)
{
	settings_init(NULL, FILENAME, NULL);
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


static void
settings_onsave_fn(
	bool* save,
	settings_save_event_data_t* data
	)
{
	*save = data->success;
}


static void
settings_onload_fn(
	bool* load,
	settings_load_event_data_t* data
	)
{
	*load = data->success;
}


static void
setting_change_fn(
	int64_t* change,
	setting_change_event_data_t* data
	)
{
	assert_false(strcmp(data->name, "foo"));
	*change = data->new_value.i64.value;
}


void assert_used
test_should_pass__settings_save_load(
	void
	)
{
	time_timers_t timers;
	time_timers_init(&timers);

	settings_t settings;
	settings_init(&settings, FILENAME, &timers);

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

	event_target_t change_target;
	event_target_init(&change_target);

	int64_t change = 0;
	event_listener_data_t listener_data =
	{
		.fn = (event_fn_t) setting_change_fn,
		.data = &change
	};
	event_listener_t* listener = event_target_add(&change_target, listener_data);

	setting_t setting_foo =
	{
		.type = SETTING_TYPE_I64,
		.value = { .i64 = { .value = 42 } },
		.constraint = { .i64 = { .min = 0, .max = 100 } },
		.change_target = &change_target
	};
	settings_add(&settings, "foo", setting_foo);
	assert_eq(change, 0);
	assert_false(saved);
	assert_false(loaded);

	setting_t setting_bar =
	{
		.type = SETTING_TYPE_F32,
		.value = { .f32 = { .value = 1.23f } },
		.constraint = { .f32 = { .min = 0.0f, .max = 2.0f } },
		.change_target = NULL
	};
	settings_add(&settings, "bar", setting_bar);
	assert_eq(change, 0);
	assert_false(saved);
	assert_false(loaded);

	settings_save(&settings);
	assert_eq(change, 0);
	assert_false(saved);
	assert_false(loaded);

	settings_modify(&settings, "foo", (setting_value_t){ .i64 = { .value = 200 } });
	assert_eq(change, 100);
	assert_false(saved);
	assert_false(loaded);

	change = 0;
	settings_modify(&settings, "foo", (setting_value_t){ .i64 = { .value = 43 } });
	assert_eq(change, 43);
	assert_false(saved);
	assert_false(loaded);

	change = 0;
	settings_save(&settings);
	assert_eq(change, 0);
	assert_true(saved);
	saved = false;
	assert_false(loaded);

	event_target_del(&settings.load_target, load_listener);
	event_target_del(&settings.save_target, save_listener);

	settings_free(&settings);

	settings_init(&settings, FILENAME, &timers);
	assert_eq(change, 0);

	save_listener = event_target_add(&settings.save_target, save_listener_data);
	load_listener = event_target_add(&settings.load_target, load_listener_data);

	settings_add(&settings, "foo", setting_foo);
	assert_eq(change, 0);
	assert_false(saved);
	assert_false(loaded);

	settings_load(&settings);
	assert_eq(change, 43);
	assert_false(saved);
	assert_true(loaded);
	loaded = false;

	event_target_del(&change_target, listener);
	event_target_free(&change_target);

	event_target_del(&settings.load_target, load_listener);
	event_target_del(&settings.save_target, save_listener);

	settings_free(&settings);

	time_timers_free(&timers);
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
test_should_fail__settings_add_null_settings(
	void
	)
{
	settings_t settings;
	settings_init(&settings, FILENAME, NULL);

	setting_t setting_foo =
	{
		.type = SETTING_TYPE_I64,
		.value = { .i64 = { .value = 42 } },
		.constraint = { .i64 = { .min = 0, .max = 100 } },
		.change_target = NULL
	};
	settings_add(NULL, "foo", setting_foo);
}


void assert_used
test_should_fail__settings_add_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, FILENAME, NULL);

	setting_t setting_foo =
	{
		.type = SETTING_TYPE_I64,
		.value = { .i64 = { .value = 42 } },
		.constraint = { .i64 = { .min = 0, .max = 100 } },
		.change_target = NULL
	};
	settings_add(&settings, NULL, setting_foo);
}


void assert_used
test_should_fail__settings_add_invalid_setting(
	void
	)
{
	settings_t settings;
	settings_init(&settings, FILENAME, NULL);

	settings_add(&settings, "foo", (setting_t){ .type = 0xFF });
}


void assert_used
test_should_fail__settings_add_null(
	void
	)
{
	settings_add(NULL, NULL, (setting_t){ .type = 0xFF });
}


void assert_used
test_should_fail__settings_modify_null_settings(
	void
	)
{
	settings_modify(NULL, "foo", (setting_value_t){ .i64 = { .value = 42 } });
}


void assert_used
test_should_fail__settings_modify_null_name(
	void
	)
{
	settings_t settings;
	settings_init(&settings, FILENAME, NULL);

	settings_modify(&settings, NULL, (setting_value_t){ .i64 = { .value = 42 } });
}


void assert_used
test_should_fail__settings_modify_null(
	void
	)
{
	settings_modify(NULL, NULL, (setting_value_t){ .i64 = { .value = 42 } });
}


void assert_used
test_should_fail__settings_add_violating_constraint(
	void
	)
{
	settings_t settings;
	settings_init(&settings, FILENAME, NULL);

	setting_t setting_foo =
	{
		.type = SETTING_TYPE_I64,
		.value = { .i64 = { .value = 42 } },
		.constraint = { .i64 = { .min = 0, .max = 40 } },
		.change_target = NULL
	};
	settings_add(&settings, "foo", setting_foo);
}
