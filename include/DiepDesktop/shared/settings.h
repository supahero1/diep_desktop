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
	struct
	{
		int64_t value;
	}
	i64;

	struct
	{
		float value;
	}
	f32;

	struct
	{
		bool value;
	}
	boolean;

	struct
	{
		uint8_t* str;
		uint64_t len;
	}
	str;

	struct
	{
		color_argb_t argb;
	}
	color;
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
	const char* name;
	setting_value_t old_value;
	setting_value_t new_value;
}
setting_change_event_data_t;

typedef struct setting
{
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
	sync_mtx_t mtx;
	hash_table_t table;
	bool dirty;

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
settings_add(
	settings_t* settings,
	const char* name,
	setting_t* setting
	);


extern void
settings_modify(
	settings_t* settings,
	const char* name,
	setting_value_t value
	);
