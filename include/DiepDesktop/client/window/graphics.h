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

#include <DiepDesktop/shared/color.h>
#include <DiepDesktop/client/tex/base.h>
#include <DiepDesktop/client/window/base.h>


typedef struct graphics_draw_data
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
graphics_draw_data_t;

typedef struct graphics_impl graphics_impl_t;

typedef struct graphics
{
	graphics_impl_t* impl;

	event_target_t draw_target;
}
graphics_t;


extern void
graphics_init(
	graphics_t* graphics,
	window_t* window
	);
