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

#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/client/window/base.h>


private void
window_sdl_assert_unreachable(
	void
	)
{
	const char* str = SDL_GetError();
	if(str)
	{
		assert_log("%s\n", str);
	}
}


private assert_ctor void
window_sdl_init(
	void
	)
{
	int status = SDL_InitSubSystem(SDL_INIT_VIDEO);
	if(!status)
	{
		window_sdl_assert_unreachable();
		hard_assert_unreachable();
	}
}


private assert_dtor void
window_sdl_free(
	void
	)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


void
window_init(
	window_t* window
	)
{
	sync_mtx_init(&window->mtx);


	SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	if(!cursor)
	{
		goto goto_sdl_err;
	}
	window->cursors[WINDOW_CURSOR_DEFAULT] = cursor;

	cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
	if(!cursor)
	{
		goto goto_sdl_err;
	}
	window->cursors[WINDOW_CURSOR_TYPING] = cursor;

	cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	if(!cursor)
	{
		goto goto_sdl_err;
	}
	window->cursors[WINDOW_CURSOR_POINTING] = cursor;

	window->current_cursor = WINDOW_CURSOR_DEFAULT;


	uint32_t props = SDL_CreateProperties();
	if(!props)
	{
		goto goto_sdl_err;
	}

	window->props = props;

	int status = SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
	if(!status)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
	if(!status)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	if(!status)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
	if(!status)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 1280);
	if(!status)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 720);
	if(!status)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Game");
	if(!status)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetBooleanProperty(props, SDL_HINT_FORCE_RAISEWINDOW, true);
	if(!status)
	{
		goto goto_sdl_err;
	}


	window->window = SDL_CreateWindowWithProperties(props);
	if(!window->window)
	{
		goto goto_sdl_err;
	}

	status = SDL_SetWindowMinimumSize(window->window, 480, 270);
	assert_true(status);

	// SDL_Rect MouseBound = { 0, 0, 1280, 720 };
	// SDL_SetWindowMouseRect(Window, &MouseBound);

	window->size = (pair_t){{ 1280, 720 }};
	window->mouse = (pair_t){{ 0, 0 }};

	window->fullscreen = false;
	window->first_frame = true;

	event_target_init(&window->resize_target);
	event_target_init(&window->focus_target);
	event_target_init(&window->blur_target);
	event_target_init(&window->key_down_target);
	event_target_init(&window->key_up_target);
	event_target_init(&window->text_target);
	event_target_init(&window->mouse_down_target);
	event_target_init(&window->mouse_up_target);
	event_target_init(&window->mouse_move_target);
	event_target_init(&window->mouse_scroll_target);

	return;


	goto_sdl_err:
	window_sdl_assert_unreachable();
	hard_assert_unreachable();
}


void
window_free(
	window_t* window
	)
{
	event_target_free(&window->mouse_scroll_target);
	event_target_free(&window->mouse_move_target);
	event_target_free(&window->mouse_up_target);
	event_target_free(&window->mouse_down_target);
	event_target_free(&window->text_target);
	event_target_free(&window->key_up_target);
	event_target_free(&window->key_down_target);
	event_target_free(&window->blur_target);
	event_target_free(&window->focus_target);
	event_target_free(&window->resize_target);

	SDL_DestroyWindow(window->window);
	SDL_DestroyProperties(window->props);

	SDL_DestroyCursor(window->cursors[WINDOW_CURSOR_POINTING]);
	SDL_DestroyCursor(window->cursors[WINDOW_CURSOR_TYPING]);
	SDL_DestroyCursor(window->cursors[WINDOW_CURSOR_DEFAULT]);

	sync_mtx_free(&window->mtx);
}


void
window_lock(
	window_t* window
	)
{
	sync_mtx_lock(&window->mtx);
}


void
window_unlock(
	window_t* window
	)
{
	sync_mtx_unlock(&window->mtx);
}


void
window_set_cursor(
	window_t* window,
	window_cursor_t cursor
	)
{
	assert_ge(cursor, 0);
	assert_lt(cursor, WINDOW_CURSOR__COUNT);

	if(window->current_cursor == cursor)
	{
		return;
	}

	window->current_cursor = cursor;
	SDL_SetCursor(window->cursors[cursor]);
}


void
window_start_typing(
	window_t* window
	)
{
	SDL_StartTextInput(window->window);
}


void
window_stop_typing(
	window_t* window
	)
{
	SDL_StopTextInput(window->window);
}


char*
window_get_clipboard(
	window_t* window,
	uint32_t* len
	)
{
	char* str = SDL_GetClipboardText();
	if(!str)
	{
		return NULL;
	}

	*len = strlen(str) + 1;

	char* new_text = alloc_malloc(*len);
	if(!new_text)
	{
		SDL_free(str);
		return NULL;
	}

	(void) memcpy(new_text, str, *len);

	SDL_free(str);

	return new_text;
}


void
window_set_clipboard(
	window_t* window,
	const char* str
	)
{
	SDL_SetClipboardText(str);
}




bool
window_is_running(
	window_t* window
	)
{
	return atomic_load_explicit(&window->running, memory_order_acquire);
}


private void
window_stop_running(
	window_t* window
	)
{
	atomic_store_explicit(&window->running, false, memory_order_release);
}


private void
window_quit(
	window_t* window
	)
{
	SDL_EVENT_USER
	SDL_RegisterEvents
	SDL_EventType a;
	SDL_Event event =
	{
		.type = SDL_EVENT_QUIT
	};
	SDL_PushEvent(&event);
}


private void
window_int_handler(
	int signal
	)
{
	window_quit();
}


private void
window_process_event(
	SDL_Event event
	)
{

}


void
window_run(
	window_t* window
	)
{
	while(window_is_running(window))
	{
		SDL_Event event;
		SDL_WaitEvent(&event);

		if(event.type == SDL_EVENT_QUIT)
		{
			window_stop_running(window);
		}

		sync_mtx_lock(&window->mtx);
			window_process_event(event);
		sync_mtx_unlock(&window->mtx);
	}
}
