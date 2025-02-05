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

#include <DiepDesktop/client/window/base.h>


typedef struct graphics
{
	window_t* window;

	event_target_t draw_target;
}
graphics_t;


extern void
graphics_init(
	graphics_t* graphics,
	window_t* window
	);


extern void
graphics_free(
	graphics_t* graphics
	);
