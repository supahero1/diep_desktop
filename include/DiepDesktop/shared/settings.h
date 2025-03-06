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

#include <DiepDesktop/shared/hash.h>
#include <DiepDesktop/shared/time.h>
#include <DiepDesktop/shared/color.h>
#include <DiepDesktop/shared/event.h>
#include <DiepDesktop/shared/macro.h>


typedef enum setting_type : uint8_t
{
	SETTING_TYPE_I64,
	SETTING_TYPE_F32,
	SETTING_TYPE_BOOLEAN,
	SETTING_TYPE_STR,
	SETTING_TYPE_COLOR,
	MACRO_ENUM_BITS(SETTING_TYPE)
}
setting_type_t;


typedef union setting_value
{
	int64_t i64;
	float f32;
	bool boolean;
	str_t str;
	color_argb_t color;
}
setting_value_t;


typedef union setting_constraint
{
	struct
	{
		int64_t min;
		int64_t max;
	}
	i64;

	struct
	{
		float min;
		float max;
	}
	f32;

	struct
	{
		uint64_t max_len;
	}
	str;
}
setting_constraint_t;


typedef struct settings settings_t;

typedef struct setting_change_event_data
{
	settings_t* settings;
	setting_value_t old_value;
	setting_value_t new_value;
}
setting_change_event_data_t;

typedef struct setting
{
	sync_rwlock_t rwlock;
	setting_type_t type;
	setting_value_t value;
	setting_constraint_t constraint;
	event_target_t* change_target;
}
setting_t;


typedef struct settings_save_event_data
{
	settings_t* settings;
	bool success;
}
settings_save_event_data_t;

typedef struct settings_load_event_data
{
	settings_t* settings;
	bool success;
}
settings_load_event_data_t;


struct settings
{
	sync_rwlock_t rwlock;

	hash_table_t table;
	bool dirty;
	bool use_timers;
	bool sealed;

	const char* path;

	time_timers_t* timers;
	time_timer_t save_timer;

	event_target_t save_target;
	event_target_t load_target;
};


extern void
settings_init(
	settings_t* settings,
	const char* path,
	time_timers_t* timers
	);


extern void
settings_free(
	settings_t* settings
	);


extern void
settings_save(
	settings_t* settings
	);


extern void
settings_load(
	settings_t* settings
	);


extern void
settings_seal(
	settings_t* settings
	);


extern setting_t*
settings_add_i64(
	settings_t* settings,
	const char* name,
	int64_t value,
	int64_t min,
	int64_t max,
	event_target_t* change_target
	);


extern setting_t*
settings_add_f32(
	settings_t* settings,
	const char* name,
	float value,
	float min,
	float max,
	event_target_t* change_target
	);


extern setting_t*
settings_add_boolean(
	settings_t* settings,
	const char* name,
	bool value,
	event_target_t* change_target
	);


extern setting_t*
settings_add_str(
	settings_t* settings,
	const char* name,
	str_t value,
	uint64_t max_len,
	event_target_t* change_target
	);


extern setting_t*
settings_add_color(
	settings_t* settings,
	const char* name,
	color_argb_t value,
	event_target_t* change_target
	);


extern void
settings_modify_i64(
	settings_t* settings,
	setting_t* setting,
	int64_t value
	);


extern void
settings_modify_f32(
	settings_t* settings,
	setting_t* setting,
	float value
	);


extern void
settings_modify_boolean(
	settings_t* settings,
	setting_t* setting,
	bool value
	);


extern void
settings_modify_str(
	settings_t* settings,
	setting_t* setting,
	str_t value
	);


extern void
settings_modify_color(
	settings_t* settings,
	setting_t* setting,
	color_argb_t value
	);


extern int64_t
setting_get_i64(
	setting_t* setting
	);


extern float
setting_get_f32(
	setting_t* setting
	);


extern bool
setting_get_boolean(
	setting_t* setting
	);


extern str_t
setting_get_str(
	setting_t* setting
	);


extern color_argb_t
setting_get_color(
	setting_t* setting
	);
