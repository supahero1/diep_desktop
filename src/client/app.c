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

#include <DiepDesktop/client/app.h>
#include <DiepDesktop/shared/time.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/settings.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/client/window/base.h>

#include <stdio.h>


struct app
{
	event_listener_t* window_close_once_listener;

	time_timers_t timers;
	settings_t settings;

	event_listener_t* settings_load_listener;
	event_listener_t* settings_save_listener;

	window_manager_t manager;
	window_t window;
};


private void
app_window_close_once_fn(
	app_t* app,
	window_close_event_data_t* event_data
	)
{
	app->window_close_once_listener = NULL;
	window_close(event_data->window);
}


private void
app_window_free_once_fn(
	app_t* app,
	window_free_event_data_t* event_data
	)
{
	if(app->window_close_once_listener)
	{
		event_target_del_once(&app->window.close_target, app->window_close_once_listener);
	}
}


private void
app_settings_load_fn(
	settings_t* settings,
	settings_load_event_data_t* event_data
	)
{
	printf("settings_load() = %d\n", event_data->success);
}


private void
app_settings_save_fn(
	settings_t* settings,
	settings_save_event_data_t* event_data
	)
{
	printf("settings_save() = %d\n", event_data->success);
}


private void
app_setting_change_fn(
	app_t* app,
	setting_change_event_data_t* event_data
	)
{
	printf("setting_change() = %s\n", event_data->name);
}


app_t*
app_init(
	void
	)
{
	app_t* app = alloc_malloc(sizeof(*app));
	assert_ptr(app, sizeof(*app));

	time_timers_init(&app->timers);
	settings_init(&app->settings, "settings.bin", &app->timers);

	event_listener_data_t settings_load_data =
	{
		.fn = (event_fn_t) app_settings_load_fn,
		.data = &app->settings
	};
	app->settings_load_listener = event_target_add(&app->settings.load_target, settings_load_data);

	event_listener_data_t settings_save_data =
	{
		.fn = (event_fn_t) app_settings_save_fn,
		.data = &app->settings
	};
	app->settings_save_listener = event_target_add(&app->settings.save_target, settings_save_data);

	window_manager_init(&app->manager);
	window_init(&app->window, &app->manager);
	window_show(&app->window);

	event_listener_data_t close_once_data =
	{
		.fn = (event_fn_t) app_window_close_once_fn,
		.data = app
	};
	app->window_close_once_listener = event_target_once(&app->window.close_target, close_once_data);

	event_listener_data_t free_once_data =
	{
		.fn = (event_fn_t) app_window_free_once_fn,
		.data = app
	};
	event_target_once(&app->window.free_target, free_once_data);

	return app;
}


void
app_free(
	app_t* app
	)
{
	window_free(&app->window);
	window_manager_free(&app->manager);

	event_target_del(&app->settings.save_target, app->settings_save_listener);
	event_target_del(&app->settings.load_target, app->settings_load_listener);

	settings_free(&app->settings);
	time_timers_free(&app->timers);

	alloc_free(sizeof(*app), app);
}


void
app_run(
	app_t* app
	)
{
	setting_value_t val = { .str = { .str = (void*) "uwu", .len = 4 } };
	setting_constraint_t constraint = { .str = { .max_len = 512 } };
	event_target_t change_target;
	event_target_init(&change_target);
	setting_t setting = { .type = SETTING_TYPE_STR, .value = val, .constraint = constraint, .change_target = &change_target };

	event_listener_data_t setting_change_data =
	{
		.fn = (event_fn_t) app_setting_change_fn,
		.data = app
	};
	event_listener_t* listener = event_target_add(&change_target, setting_change_data);

	settings_add(&app->settings, "hewwo", &setting);
	// app->settings.dirty = true;
	// settings_save(&app->settings);
	settings_load(&app->settings);

	window_manager_run(&app->manager);

	event_target_del(&change_target, listener);
}
