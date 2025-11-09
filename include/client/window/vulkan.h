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

#include <shared/color.h>
#include <client/tex/base.h>
#include <client/window/base.h>


typedef struct vulkan_draw_data
{
	pair_t       pos;
	pair_t       size;
	float        angle;
	color_argb_t white_color;
	float        white_depth;
	color_argb_t black_color;
	float        black_depth;
	tex_t        tex;
	pair_t       tex_scale;
	pair_t       tex_offset;
}
vulkan_draw_data_t;


typedef struct vulkan_draw_event_data
{
	float delta;
	float fps;
}
vulkan_draw_event_data_t;

typedef struct vulkan_event_table
{
	event_target_t draw_target;
}
vulkan_event_table_t;

typedef struct vulkan* vulkan_t;


extern vulkan_t
vulkan_init(
	window_t window
	);


extern vulkan_event_table_t*
vulkan_get_event_table(
	vulkan_t vk
	);


extern void
vulkan_add_draw_data(
	vulkan_t vk,
	const vulkan_draw_data_t* data
	);


extern void
vulkan_set_buffering(
	vulkan_t vk,
	uint32_t buffering
	);


extern float
vulkan_get_fps(
	vulkan_t vk
	);
