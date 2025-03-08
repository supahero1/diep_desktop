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
#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/time.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/settings.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/client/window/base.h>

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>


struct app
{
	event_listener_t* window_close_once_listener;
	event_listener_t* window_move_listener;
	event_listener_t* window_resize_listener;
	event_listener_t* window_fullscreen_listener;

	time_timers_t timers;
	settings_t settings;

	setting_t* window_pos_x;
	setting_t* window_pos_y;
	setting_t* window_w;
	setting_t* window_h;
	setting_t* window_fullscreen;

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
	event_target_del(&app->window.fullscreen_target, app->window_fullscreen_listener);
	event_target_del(&app->window.resize_target, app->window_resize_listener);
	event_target_del(&app->window.move_target, app->window_move_listener);

	if(app->window_close_once_listener)
	{
		event_target_del_once(&app->window.close_target, app->window_close_once_listener);
	}
}


private void
app_window_on_move_fn(
	app_t* app,
	window_move_event_data_t* event_data
	)
{
	settings_modify_f32(&app->settings, app->window_pos_x, event_data->new_pos.x);
	settings_modify_f32(&app->settings, app->window_pos_y, event_data->new_pos.y);
}


private void
app_window_on_resize_fn(
	app_t* app,
	window_resize_event_data_t* event_data
	)
{
	settings_modify_f32(&app->settings, app->window_w, event_data->new_size.w);
	settings_modify_f32(&app->settings, app->window_h, event_data->new_size.h);

	if(event_data->new_size.w >= 1600.0f)
	{
		window_close(&app->window);
	}
}


private void
app_window_on_fullscreen_fn(
	app_t* app,
	window_focus_event_data_t* event_data
	)
{
	settings_modify_boolean(&app->settings, app->window_fullscreen, true);
}


app_t*
app_init(
	int argc,
	char** argv
	)
{
	app_t* app = alloc_malloc(sizeof(*app));
	assert_ptr(app, sizeof(*app));

	assert_ge(argc, 1);
	assert_not_null(argv);
	assert_not_null(argv[0]);

	char exe_path[PATH_MAX];
	realpath(argv[0], exe_path);
	char* dir = dirname(exe_path);

	int status = chdir(dir);
	hard_assert_eq(status, 0);

	dir_create("settings");

	time_timers_init(&app->timers);
	settings_init(&app->settings, "settings/window.bin", &app->timers);

	app->window_pos_x = settings_add_f32(&app->settings, "main_window_pos_x", 0.0f, -16384.0f, 16384.0f, NULL);
	app->window_pos_y = settings_add_f32(&app->settings, "main_window_pos_y", 0.0f, -16384.0f, 16384.0f, NULL);
	app->window_w = settings_add_f32(&app->settings, "main_window_w", 1280.0f, 480.0f, 16384.0f, NULL);
	app->window_h = settings_add_f32(&app->settings, "main_window_h", 720.0f, 270.0f, 16384.0f, NULL);
	app->window_fullscreen = settings_add_boolean(&app->settings, "main_window_fullscreen", false, NULL);

	settings_seal(&app->settings);

	settings_load(&app->settings);

	window_history_t history =
	{
		.extent =
		{
			.x = setting_get_f32(app->window_pos_x),
			.y = setting_get_f32(app->window_pos_y),
			.w = setting_get_f32(app->window_w),
			.h = setting_get_f32(app->window_h)
		},
		.fullscreen = setting_get_boolean(app->window_fullscreen)
	};

	window_manager_init(&app->manager);
	window_init(&app->window);
	window_manager_add(&app->manager, &app->window, "Test", &history);
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

	event_listener_data_t move_data =
	{
		.fn = (event_fn_t) app_window_on_move_fn,
		.data = app
	};
	app->window_move_listener = event_target_add(&app->window.move_target, move_data);

	event_listener_data_t resize_data =
	{
		.fn = (event_fn_t) app_window_on_resize_fn,
		.data = app
	};
	app->window_resize_listener = event_target_add(&app->window.resize_target, resize_data);

	event_listener_data_t fullscreen_data =
	{
		.fn = (event_fn_t) app_window_on_fullscreen_fn,
		.data = app
	};
	app->window_fullscreen_listener = event_target_add(&app->window.fullscreen_target, fullscreen_data);

	// ui

	return app;
}


void
app_free(
	app_t* app
	)
{
	assert_not_null(app);

	window_manager_free(&app->manager);

	settings_free(&app->settings);
	time_timers_free(&app->timers);

	alloc_free(app, sizeof(*app));
}


void
app_run(
	app_t* app
	)
{
	assert_not_null(app);

	window_manager_run(&app->manager);
}
