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

#include <DiepDesktop/client/window/base.h>

#include <stdio.h>


void
window_close_fn(
	void* data,
	window_close_event_data_t* event_data
	)
{
	window_free(event_data->window);
}


int
main(
	void
	)
{
	window_manager_t manager;
	window_manager_init(&manager);

	window_t window1;
	window_init(&window1, &manager);
	window_show(&window1);
	event_target_once(&window1.close_target,
		(event_listener_data_t)
		{
			.fn = (event_fn_t) window_close_fn,
			.data = NULL
		}
		);

	window_manager_run(&manager);

	window_manager_free(&manager);

	return 0;
}
