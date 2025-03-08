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

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdatomic.h>


#define ___(x) WINDOW_KEY_##x
#define __(x) SDLK_##x
#define _(x) case __(x): return ___(x);

private window_key_t
window_map_sdl_key(
	int sdl_key
	)
{
	switch(sdl_key)
	{

	_(UNKNOWN)_(RETURN)_(ESCAPE)_(BACKSPACE)_(TAB)_(SPACE)_(EXCLAIM)_(DBLAPOSTROPHE)_(HASH)_(DOLLAR)_(PERCENT)
	_(AMPERSAND)_(APOSTROPHE)_(LEFTPAREN)_(RIGHTPAREN)_(ASTERISK)_(PLUS)_(COMMA)_(MINUS)_(PERIOD)_(SLASH)_(0)_(1)_(2)
	_(3)_(4)_(5)_(6)_(7)_(8)_(9)_(COLON)_(SEMICOLON)_(LESS)_(EQUALS)_(GREATER)_(QUESTION)_(AT)_(LEFTBRACKET)_(BACKSLASH)
	_(RIGHTBRACKET)_(CARET)_(UNDERSCORE)_(GRAVE)_(A)_(B)_(C)_(D)_(E)_(F)_(G)_(H)_(I)_(J)_(K)_(L)_(M)_(N)_(O)_(P)_(Q)_(R)
	_(S)_(T)_(U)_(V)_(W)_(X)_(Y)_(Z)_(LEFTBRACE)_(PIPE)_(RIGHTBRACE)_(TILDE)_(DELETE)_(PLUSMINUS)_(CAPSLOCK)_(F1)_(F2)
	_(F3)_(F4)_(F5)_(F6)_(F7)_(F8)_(F9)_(F10)_(F11)_(F12)_(PRINTSCREEN)_(SCROLLLOCK)_(PAUSE)_(INSERT)_(HOME)_(PAGEUP)
	_(END)_(PAGEDOWN)_(RIGHT)_(LEFT)_(DOWN)_(UP)_(NUMLOCKCLEAR)_(KP_DIVIDE)_(KP_MULTIPLY)_(KP_MINUS)_(KP_PLUS)
	_(KP_ENTER)_(KP_1)_(KP_2)_(KP_3)_(KP_4)_(KP_5)_(KP_6)_(KP_7)_(KP_8)_(KP_9)_(KP_0)_(KP_PERIOD)_(APPLICATION)_(POWER)
	_(KP_EQUALS)_(F13)_(F14)_(F15)_(F16)_(F17)_(F18)_(F19)_(F20)_(F21)_(F22)_(F23)_(F24)_(EXECUTE)_(HELP)_(MENU)
	_(SELECT)_(STOP)_(AGAIN)_(UNDO)_(CUT)_(COPY)_(PASTE)_(FIND)_(MUTE)_(VOLUMEUP)_(VOLUMEDOWN)_(KP_COMMA)
	_(KP_EQUALSAS400)_(ALTERASE)_(SYSREQ)_(CANCEL)_(CLEAR)_(PRIOR)_(RETURN2)_(SEPARATOR)_(OUT)_(OPER)_(CLEARAGAIN)
	_(CRSEL)_(EXSEL)_(KP_00)_(KP_000)_(THOUSANDSSEPARATOR)_(DECIMALSEPARATOR)_(CURRENCYUNIT)_(CURRENCYSUBUNIT)
	_(KP_LEFTPAREN)_(KP_RIGHTPAREN)_(KP_LEFTBRACE)_(KP_RIGHTBRACE)_(KP_TAB)_(KP_BACKSPACE)_(KP_A)_(KP_B)_(KP_C)_(KP_D)
	_(KP_E)_(KP_F)_(KP_XOR)_(KP_POWER)_(KP_PERCENT)_(KP_LESS)_(KP_GREATER)_(KP_AMPERSAND)_(KP_DBLAMPERSAND)
	_(KP_VERTICALBAR)_(KP_DBLVERTICALBAR)_(KP_COLON)_(KP_HASH)_(KP_SPACE)_(KP_AT)_(KP_EXCLAM)_(KP_MEMSTORE)
	_(KP_MEMRECALL)_(KP_MEMCLEAR)_(KP_MEMADD)_(KP_MEMSUBTRACT)_(KP_MEMMULTIPLY)_(KP_MEMDIVIDE)_(KP_PLUSMINUS)_(KP_CLEAR)
	_(KP_CLEARENTRY)_(KP_BINARY)_(KP_OCTAL)_(KP_DECIMAL)_(KP_HEXADECIMAL)_(LCTRL)_(LSHIFT)_(LALT)_(LGUI)_(RCTRL)
	_(RSHIFT)_(RALT)_(RGUI)_(MODE)_(SLEEP)_(WAKE)_(CHANNEL_INCREMENT)_(CHANNEL_DECREMENT)_(MEDIA_PLAY)_(MEDIA_PAUSE)
	_(MEDIA_RECORD)_(MEDIA_FAST_FORWARD)_(MEDIA_REWIND)_(MEDIA_NEXT_TRACK)_(MEDIA_PREVIOUS_TRACK)_(MEDIA_STOP)
	_(MEDIA_EJECT)_(MEDIA_PLAY_PAUSE)_(MEDIA_SELECT)_(AC_NEW)_(AC_OPEN)_(AC_CLOSE)_(AC_EXIT)_(AC_SAVE)_(AC_PRINT)
	_(AC_PROPERTIES)_(AC_SEARCH)_(AC_HOME)_(AC_BACK)_(AC_FORWARD)_(AC_STOP)_(AC_REFRESH)_(AC_BOOKMARKS)_(SOFTLEFT)
	_(SOFTRIGHT)_(CALL)_(ENDCALL)

	default: return WINDOW_KEY_UNKNOWN;

	}
}

#undef _
#undef __
#undef ___


private window_mod_t
window_map_sdl_mod(
	int sdl_mods
	)
{
	window_mod_t mods = 0;

	if(sdl_mods & SDL_KMOD_SHIFT) mods |= WINDOW_MOD_SHIFT_BIT;
	if(sdl_mods & SDL_KMOD_CTRL ) mods |= WINDOW_MOD_CTRL_BIT;
	if(sdl_mods & SDL_KMOD_ALT  ) mods |= WINDOW_MOD_ALT_BIT;
	if(sdl_mods & SDL_KMOD_GUI  ) mods |= WINDOW_MOD_GUI_BIT;
	if(sdl_mods & SDL_KMOD_CAPS ) mods |= WINDOW_MOD_CAPS_LOCK_BIT;

	return mods;
}


private window_button_t
window_map_sdl_button(
	int button
	)
{
	switch(button)
	{

	case SDL_BUTTON_LEFT: return WINDOW_BUTTON_LEFT;
	case SDL_BUTTON_MIDDLE: return WINDOW_BUTTON_MIDDLE;
	case SDL_BUTTON_RIGHT: return WINDOW_BUTTON_RIGHT;
	case SDL_BUTTON_X1: return WINDOW_BUTTON_X1;
	case SDL_BUTTON_X2: return WINDOW_BUTTON_X2;
	default: return WINDOW_BUTTON_UNKNOWN;

	}
}


private void
window_sdl_log_error(
	void
	)
{
	const char* str = SDL_GetError();
	if(str)
	{
		fprintf(stderr, "SDL_GetError: '%s'\n", str);
	}
}


private assert_ctor void
window_sdl_init(
	void
	)
{
	bool status = SDL_InitSubSystem(SDL_INIT_VIDEO);
	hard_assert_true(status, window_sdl_log_error());
}


private assert_dtor void
window_sdl_free(
	void
	)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


private void
window_free_once_fn(
	window_t* window,
	window_free_event_data_t* event_data
	)
{
	event_target_free(&window->mouse_scroll_target);
	event_target_free(&window->mouse_move_target);
	event_target_free(&window->mouse_up_target);
	event_target_free(&window->mouse_down_target);
	event_target_free(&window->set_clipboard_target);
	event_target_free(&window->get_clipboard_target);
	event_target_free(&window->text_target);
	event_target_free(&window->key_up_target);
	event_target_free(&window->key_down_target);
	event_target_free(&window->fullscreen_target);
	event_target_free(&window->close_target);
	event_target_free(&window->blur_target);
	event_target_free(&window->focus_target);
	event_target_free(&window->resize_target);
	event_target_free(&window->move_target);
	event_target_free(&window->free_target);
	event_target_free(&window->init_target);
}


struct window_impl
{
	window_manager_t* manager;

	window_t* next;
	window_t* prev;

	SDL_PropertiesID sdl_props;
	SDL_Window* sdl_window;
	SDL_PropertiesID props;

	sync_mtx_t mtx;

	window_info_t info;
};


void
window_init(
	window_t* window
	)
{
	assert_not_null(window);

	window->impl = alloc_malloc(sizeof(*window->impl));
	assert_not_null(window->impl);

	event_target_init(&window->init_target);
	event_target_init(&window->free_target);
	event_target_init(&window->move_target);
	event_target_init(&window->resize_target);
	event_target_init(&window->focus_target);
	event_target_init(&window->blur_target);
	event_target_init(&window->close_target);
	event_target_init(&window->fullscreen_target);
	event_target_init(&window->key_down_target);
	event_target_init(&window->key_up_target);
	event_target_init(&window->text_target);
	event_target_init(&window->get_clipboard_target);
	event_target_init(&window->set_clipboard_target);
	event_target_init(&window->mouse_down_target);
	event_target_init(&window->mouse_up_target);
	event_target_init(&window->mouse_move_target);
	event_target_init(&window->mouse_scroll_target);

	event_listener_data_t free_data =
	{
		.fn = (event_fn_t) window_free_once_fn,
		.data = window
	};
	(void) event_target_once(&window->free_target, free_data);
}


private void
window_free(
	window_t* window
	)
{
	assert_not_null(window);

	window_free_event_data_t event_data =
	{
		.window = window
	};
	event_target_fire(&window->free_target, &event_data);

	SDL_DestroyWindow(window->impl->sdl_window);
	SDL_DestroyProperties(window->impl->sdl_props);

	alloc_free(window->impl, sizeof(*window->impl));
}


void
window_close(
	window_t* window
	)
{
	assert_not_null(window);

	window_user_event_window_close_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_window_close_data_t)
	{
	};

	window_push_event(window, WINDOW_USER_EVENT_WINDOW_CLOSE, data);
}


void
window_set(
	window_t* window,
	const char* name,
	void* data
	)
{
	assert_not_null(window);
	assert_not_null(name);

	bool status = SDL_SetPointerProperty(window->impl->props, name, window);
	hard_assert_true(status, window_sdl_log_error());
}


void*
window_get(
	window_t* window,
	const char* name
	)
{
	assert_not_null(window);
	assert_not_null(name);

	return SDL_GetPointerProperty(window->impl->props, name, NULL);
}


void
window_push_event(
	window_t* window,
	window_user_event_t type,
	void* data
	)
{
	assert_not_null(window);

	window_manager_push_event(window->impl->manager, type, window, data);
}


void
window_set_cursor(
	window_t* window,
	window_cursor_t cursor
	)
{
	assert_not_null(window);
	assert_ge(cursor, 0);
	assert_lt(cursor, WINDOW_CURSOR__COUNT);

	window_user_event_set_cursor_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_set_cursor_data_t)
	{
		.cursor = cursor
	};

	window_push_event(window, WINDOW_USER_EVENT_SET_CURSOR, data);
}


void
window_show(
	window_t* window
	)
{
	assert_not_null(window);

	window_user_event_show_window_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_show_window_data_t)
	{
	};

	window_push_event(window, WINDOW_USER_EVENT_SHOW_WINDOW, data);
}


void
window_hide(
	window_t* window
	)
{
	assert_not_null(window);

	window_user_event_hide_window_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_hide_window_data_t)
	{
	};

	window_push_event(window, WINDOW_USER_EVENT_HIDE_WINDOW, data);
}


void
window_start_typing(
	window_t* window
	)
{
	assert_not_null(window);

	window_user_event_start_typing_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_start_typing_data_t)
	{
	};

	window_push_event(window, WINDOW_USER_EVENT_START_TYPING, data);
}


void
window_stop_typing(
	window_t* window
	)
{
	assert_not_null(window);

	window_user_event_stop_typing_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_stop_typing_data_t)
	{
	};

	window_push_event(window, WINDOW_USER_EVENT_STOP_TYPING, data);
}


void
window_get_clipboard(
	window_t* window
	)
{
	assert_not_null(window);

	window_user_event_get_clipboard_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_get_clipboard_data_t)
	{
	};

	window_push_event(window, WINDOW_USER_EVENT_GET_CLIPBOARD, data);
}


void
window_set_clipboard(
	window_t* window,
	const char* str
	)
{
	assert_not_null(window);

	window_user_event_set_clipboard_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_set_clipboard_data_t)
	{
		.str = str
	};

	window_push_event(window, WINDOW_USER_EVENT_SET_CLIPBOARD, data);
}


void
window_toggle_fullscreen(
	window_t* window
	)
{
	assert_not_null(window);

	window_user_event_window_fullscreen_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	*data =
	(window_user_event_window_fullscreen_data_t)
	{
	};

	window_push_event(window, WINDOW_USER_EVENT_WINDOW_FULLSCREEN, data);
}


void
window_get_info(
	window_t* window,
	window_info_t* info
	)
{
	assert_not_null(window);
	assert_not_null(info);

	sync_mtx_lock(&window->impl->mtx);
		*info = window->impl->info;
	sync_mtx_unlock(&window->impl->mtx);
}


private void
window_process_event(
	window_t* window,
	SDL_Event* event
	)
{
	switch(event->type)
	{

	case SDL_EVENT_WINDOW_MOVED:
	{
		window_move_event_data_t event_data =
		{
			.window = window,
			.old_pos = window->impl->info.extent.pos,
			.new_pos = {{ event->window.data1, event->window.data2 }}
		};

		sync_mtx_lock(&window->impl->mtx);
			window->impl->info.extent.pos = event_data.new_pos;
		sync_mtx_unlock(&window->impl->mtx);

		event_target_fire(&window->move_target, &event_data);

		break;
	}

	case SDL_EVENT_WINDOW_RESIZED:
	{
		window_resize_event_data_t event_data =
		{
			.window = window,
			.old_size = window->impl->info.extent.size,
			.new_size = {{ event->window.data1, event->window.data2 }}
		};

		sync_mtx_lock(&window->impl->mtx);
			window->impl->info.extent.size = event_data.new_size;
		sync_mtx_unlock(&window->impl->mtx);

		event_target_fire(&window->resize_target, &event_data);

		break;
	}

	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	{
		window_focus_event_data_t event_data =
		{
			.window = window
		};
		event_target_fire(&window->focus_target, &event_data);

		break;
	}

	case SDL_EVENT_WINDOW_FOCUS_LOST:
	{
		window_blur_event_data_t event_data =
		{
			.window = window
		};
		event_target_fire(&window->blur_target, &event_data);

		break;
	}

	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
	{
		window_close_event_data_t event_data =
		{
			.window = window
		};
		event_target_fire(&window->close_target, &event_data);

		break;
	}

	case SDL_EVENT_KEY_DOWN:
	{
		window_key_down_event_data_t event_data =
		{
			.window = window,
			.key = window_map_sdl_key(event->key.key),
			.mods = window_map_sdl_mod(event->key.mod),
			.repeat = event->key.repeat
		};
		event_target_fire(&window->key_down_target, &event_data);

		break;
	}

	case SDL_EVENT_KEY_UP:
	{
		window_key_up_event_data_t event_data =
		{
			.window = window,
			.key = window_map_sdl_key(event->key.key),
			.mods = window_map_sdl_mod(event->key.mod)
		};
		event_target_fire(&window->key_up_target, &event_data);

		break;
	}

	case SDL_EVENT_TEXT_INPUT:
	{
		if(event->text.text)
		{
			window_text_event_data_t event_data =
			{
				.window = window,
				.str = event->text.text,
				.len = strlen(event->text.text)
			};
			event_target_fire(&window->text_target, &event_data);
		}

		break;
	}

	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	{
		assert_true(event->button.down);

		window_mouse_down_event_data_t event_data =
		{
			.window = window,
			.button = window_map_sdl_button(event->button.button),
			.pos = {{ event->button.x, event->button.y }},
			.clicks = event->button.clicks
		};
		event_target_fire(&window->mouse_down_target, &event_data);

		break;
	}

	case SDL_EVENT_MOUSE_BUTTON_UP:
	{
		assert_false(event->button.down);

		window_mouse_up_event_data_t event_data =
		{
			.window = window,
			.button = window_map_sdl_button(event->button.button),
			.clicks = event->button.clicks,
			.pos = {{ event->button.x, event->button.y }}
		};
		event_target_fire(&window->mouse_up_target, &event_data);

		break;
	}

	case SDL_EVENT_MOUSE_MOTION:
	{
		window_mouse_move_event_data_t event_data =
		{
			.window = window,
			.old_pos = window->impl->info.mouse,
			.new_pos = {{ event->motion.x, event->motion.y }}
		};

		sync_mtx_lock(&window->impl->mtx);
			window->impl->info.mouse = event_data.new_pos;
		sync_mtx_unlock(&window->impl->mtx);

		event_target_fire(&window->mouse_move_target, &event_data);

		break;
	}

	case SDL_EVENT_MOUSE_WHEEL:
	{
		window_mouse_scroll_event_data_t event_data =
		{
			.window = window,
			.offset_y = event->wheel.y * (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f)
		};
		event_target_fire(&window->mouse_scroll_target, &event_data);

		break;
	}

	default: break;

	}
}





struct window_manager_impl
{
	_Atomic bool running;

	window_t* window_head;
	SDL_Cursor* cursors[WINDOW_CURSOR__COUNT];

	uint32_t window_count;
	window_cursor_t current_cursor;
};


void
window_manager_init(
	window_manager_t* manager
	)
{
	assert_not_null(manager);

	manager->impl = alloc_malloc(sizeof(*manager->impl));
	assert_not_null(manager->impl);

	atomic_init(&manager->impl->running, true);

	manager->impl->window_head = NULL;
	manager->impl->window_count = 0;

	SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	hard_assert_not_null(cursor, window_sdl_log_error());
	manager->impl->cursors[WINDOW_CURSOR_DEFAULT] = cursor;

	cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
	hard_assert_not_null(cursor, window_sdl_log_error());
	manager->impl->cursors[WINDOW_CURSOR_TYPING] = cursor;

	cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	hard_assert_not_null(cursor, window_sdl_log_error());
	manager->impl->cursors[WINDOW_CURSOR_POINTING] = cursor;

	manager->impl->current_cursor = WINDOW_CURSOR_DEFAULT;
}


void
window_manager_free(
	window_manager_t* manager
	)
{
	assert_not_null(manager);

	SDL_DestroyCursor(manager->impl->cursors[WINDOW_CURSOR_POINTING]);
	SDL_DestroyCursor(manager->impl->cursors[WINDOW_CURSOR_TYPING]);
	SDL_DestroyCursor(manager->impl->cursors[WINDOW_CURSOR_DEFAULT]);

	assert_null(manager->impl->window_head);
	assert_eq(manager->impl->window_count, 0);

	alloc_free(manager->impl, sizeof(*manager->impl));
}


void
window_manager_add(
	window_manager_t* manager,
	window_t* window,
	const char* title,
	const window_history_t* history
	)
{
	assert_not_null(manager);
	assert_not_null(window);
	assert_not_null(title);

	window->impl->manager = manager;

	window->impl->next = manager->impl->window_head;
	window->impl->prev = NULL;
	if(manager->impl->window_head)
	{
		manager->impl->window_head->impl->prev = window;
	}

	manager->impl->window_head = window;
	++manager->impl->window_count;

	window_user_event_window_init_data_t* data = alloc_malloc(sizeof(*data));
	assert_ptr(data, sizeof(*data));

	str_t title_str;
	str_init_copy_cstr(&title_str, title);

	window_history_t* history_copy;
	if(history)
	{
		history_copy = alloc_malloc(sizeof(*history_copy));
		assert_ptr(history_copy, sizeof(*history_copy));

		*history_copy = *history;
	}
	else
	{
		history_copy = NULL;
	}

	*data =
	(window_user_event_window_init_data_t)
	{
		.title = title_str,
		.history = history_copy
	};
	window_push_event(window, WINDOW_USER_EVENT_WINDOW_INIT, data);
}


void
window_manager_push_event(
	window_manager_t* manager,
	window_user_event_t type,
	void* context,
	void* data
	)
{
	assert_not_null(manager);

	SDL_Event event =
	{
		.user =
		{
			.type = SDL_EVENT_USER,
			.code = type,
			.data1 = context,
			.data2 = data
		}
	};
	SDL_PushEvent(&event);
}


bool
window_manager_is_running(
	window_manager_t* manager
	)
{
	assert_not_null(manager);

	return atomic_load_explicit(&manager->impl->running, memory_order_acquire);
}


void
window_manager_stop_running(
	window_manager_t* manager
	)
{
	assert_not_null(manager);

	atomic_store_explicit(&manager->impl->running, false, memory_order_release);
}


private void
window_manager_process_user_event(
	window_manager_t* manager,
	SDL_Event* event
	)
{
	window_t* window = event->user.data1;
	void* event_data = event->user.data2;


	switch(event->user.code)
	{

	case WINDOW_USER_EVENT_WINDOW_INIT:
	{
		window_user_event_window_init_data_t* data = event_data;

		uint32_t sdl_props = SDL_CreateProperties();
		hard_assert_neq(sdl_props, 0, window_sdl_log_error());

		window->impl->sdl_props = sdl_props;

		bool status = SDL_SetBooleanProperty(sdl_props,
			SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
		hard_assert_true(status, window_sdl_log_error());

		status = SDL_SetBooleanProperty(sdl_props,
			SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
		hard_assert_true(status, window_sdl_log_error());

		status = SDL_SetBooleanProperty(sdl_props,
			SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
		hard_assert_true(status, window_sdl_log_error());

		status = SDL_SetBooleanProperty(sdl_props,
			SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
		hard_assert_true(status, window_sdl_log_error());

		if(data->history)
		{
			status = SDL_SetNumberProperty(sdl_props,
				SDL_PROP_WINDOW_CREATE_X_NUMBER, data->history->extent.x);
			hard_assert_true(status, window_sdl_log_error());

			status = SDL_SetNumberProperty(sdl_props,
				SDL_PROP_WINDOW_CREATE_Y_NUMBER, data->history->extent.y);
			hard_assert_true(status, window_sdl_log_error());
		}

		status = SDL_SetNumberProperty(sdl_props,
			SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,
			data->history ? data->history->extent.w : 1280);
		hard_assert_true(status, window_sdl_log_error());

		status = SDL_SetNumberProperty(sdl_props,
			SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,
			data->history ? data->history->extent.h : 720);
		hard_assert_true(status, window_sdl_log_error());

		status = SDL_SetStringProperty(sdl_props,
			SDL_PROP_WINDOW_CREATE_TITLE_STRING, data->title.str);
		hard_assert_true(status, window_sdl_log_error());

		status = SDL_SetBooleanProperty(sdl_props,
			SDL_HINT_FORCE_RAISEWINDOW, true);
		hard_assert_true(status, window_sdl_log_error());


		window->impl->sdl_window = SDL_CreateWindowWithProperties(sdl_props);
		hard_assert_not_null(window->impl->sdl_window, window_sdl_log_error());

		uint32_t props = SDL_GetWindowProperties(window->impl->sdl_window);
		hard_assert_neq(props, 0, window_sdl_log_error());

		window->impl->props = props;
		window_set(window, "WINDOW_PTR", window);

		status = SDL_SetWindowMinimumSize(window->impl->sdl_window, 480, 270);
		hard_assert_true(status, window_sdl_log_error());


		ipair_t pos = {0};
		status = SDL_GetWindowPosition(window->impl->sdl_window, &pos.x, &pos.y);
		hard_assert_eq(status, true);

		window->impl->info.extent.pos = (pair_t){{ pos.x, pos.y }};

		ipair_t size = {0};
		status = SDL_GetWindowSize(window->impl->sdl_window, &size.w, &size.h);
		hard_assert_eq(status, true);

		window->impl->info.extent.size = (pair_t){{ size.w, size.h }};

		window->impl->info.mouse = (pair_t){{ 0, 0 }};

		window->impl->info.fullscreen = false;

		window_init_event_data_t event_data =
		{
			.window = window
		};
		event_target_fire(&window->init_target, &event_data);

		alloc_free(data->history, sizeof(*data->history));
		str_free(&data->title);
		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_WINDOW_FREE:
	{
		window_user_event_window_free_data_t* data = event_data;

		if(window->impl->prev)
		{
			window->impl->prev->impl->next = window->impl->next;
		}
		else
		{
			manager->impl->window_head = window->impl->next;
		}

		if(window->impl->next)
		{
			window->impl->next->impl->prev = window->impl->prev;
		}

		if(--manager->impl->window_count == 0)
		{
			window_manager_stop_running(manager);
		}

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_WINDOW_CLOSE:
	{
		window_user_event_window_close_data_t* data = event_data;

		window_free(window);

		window_user_event_window_free_data_t* free_data = alloc_malloc(sizeof(*free_data));
		assert_ptr(free_data, sizeof(*free_data));

		*free_data =
		(window_user_event_window_free_data_t)
		{
		};
		window_push_event(window, WINDOW_USER_EVENT_WINDOW_FREE, free_data);

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_WINDOW_FULLSCREEN:
	{
		window_user_event_window_fullscreen_data_t* data = event_data;

		sync_mtx_lock(&window->impl->mtx);
			window->impl->info.fullscreen = !window->impl->info.fullscreen;
		sync_mtx_unlock(&window->impl->mtx);

		if(window->impl->info.fullscreen)
		{
			ipair_t old_size;
			bool status = SDL_GetWindowSize(window->impl->sdl_window, &old_size.w, &old_size.h);
			hard_assert_true(status, window_sdl_log_error());

			window->impl->info.old_extent.size =
			(pair_t)
			{
				.w = old_size.w,
				.h = old_size.h
			};


			ipair_t old_pos;
			status = SDL_GetWindowPosition(window->impl->sdl_window, &old_pos.x, &old_pos.y);
			hard_assert_true(status, window_sdl_log_error());

			window->impl->info.old_extent.pos =
			(pair_t)
			{
				.x = old_pos.x,
				.y = old_pos.y
			};


			status = SDL_SetWindowFullscreen(window->impl->sdl_window, true);
			hard_assert_true(status, window_sdl_log_error());
		}
		else
		{
			bool status = SDL_SetWindowFullscreen(window->impl->sdl_window, false);
			hard_assert_true(status, window_sdl_log_error());

			status = SDL_SetWindowSize(window->impl->sdl_window,
				window->impl->info.old_extent.size.w, window->impl->info.old_extent.size.h);
			hard_assert_true(status, window_sdl_log_error());

			status = SDL_SetWindowPosition(window->impl->sdl_window,
				window->impl->info.old_extent.pos.x, window->impl->info.old_extent.pos.y);
			hard_assert_true(status, window_sdl_log_error());
		}

		window_fullscreen_event_data_t event_data =
		{
			.window = window,
			.fullscreen = window->impl->info.fullscreen
		};
		event_target_fire(&window->fullscreen_target, &event_data);

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_SET_CURSOR:
	{
		window_user_event_set_cursor_data_t* data = event_data;
		assert_ge(data->cursor, 0);
		assert_lt(data->cursor, WINDOW_CURSOR__COUNT);

		if(manager->impl->current_cursor != data->cursor)
		{
			manager->impl->current_cursor = data->cursor;
			SDL_SetCursor(manager->impl->cursors[data->cursor]);
		}

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_SHOW_WINDOW:
	{
		window_user_event_show_window_data_t* data = event_data;

		bool status = SDL_ShowWindow(window->impl->sdl_window);
		hard_assert_true(status, window_sdl_log_error());

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_HIDE_WINDOW:
	{
		window_user_event_hide_window_data_t* data = event_data;

		bool status = SDL_HideWindow(window->impl->sdl_window);
		hard_assert_true(status, window_sdl_log_error());

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_START_TYPING:
	{
		window_user_event_start_typing_data_t* data = event_data;

		bool status = SDL_StartTextInput(window->impl->sdl_window);
		hard_assert_true(status, window_sdl_log_error());

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_STOP_TYPING:
	{
		window_user_event_stop_typing_data_t* data = event_data;

		bool status = SDL_StopTextInput(window->impl->sdl_window);
		hard_assert_true(status, window_sdl_log_error());

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_SET_CLIPBOARD:
	{
		window_user_event_set_clipboard_data_t* data = event_data;

		bool status = SDL_SetClipboardText(data->str);
		if(!status)
		{
			window_sdl_log_error();
		}

		window_set_clipboard_event_data_t event_data =
		{
			.window = window,
			.success = status
		};
		event_target_fire(&window->set_clipboard_target, &event_data);

		alloc_free(data, sizeof(*data));

		break;
	}

	case WINDOW_USER_EVENT_GET_CLIPBOARD:
	{
		window_user_event_get_clipboard_data_t* data = event_data;

		char* str = SDL_GetClipboardText();
		if(str)
		{
			window_get_clipboard_event_data_t event_data =
			{
				.window = window,

				.str = str,
				.len = strlen(str)
			};
			event_target_fire(&window->get_clipboard_target, &event_data);

			SDL_free(str);
		}

		alloc_free(data, sizeof(*data));

		break;
	}

	default: assert_unreachable();

	}
}


private void
window_manager_process_global_event(
	window_manager_t* manager,
	SDL_Event* event
	)
{
	switch(event->type)
	{

	case SDL_EVENT_QUIT:
	{
		window_manager_stop_running(manager);
		break;
	}

	default: break;

	}
}


private void
window_manager_process_event(
	window_manager_t* manager,
	SDL_Event* event
	)
{
	if(event->type == SDL_EVENT_USER)
	{
		window_manager_process_user_event(manager, event);
	}
	else
	{
		SDL_Window* sdl_window = SDL_GetWindowFromEvent(event);
		if(!sdl_window)
		{
			window_manager_process_global_event(manager, event);
		}
		else
		{
			SDL_PropertiesID sdl_props = SDL_GetWindowProperties(sdl_window);
			assert_neq(sdl_props, 0, window_sdl_log_error());

			window_t* window = SDL_GetPointerProperty(sdl_props, "WINDOW_PTR", NULL);
			assert_not_null(window, window_sdl_log_error());

			window_process_event(window, event);
		}
	}
}


void
window_manager_run(
	window_manager_t* manager
	)
{
	assert_not_null(manager);

	while(window_manager_is_running(manager))
	{
		SDL_Event event;
		SDL_WaitEvent(&event);

		if(event.type == SDL_EVENT_QUIT)
		{
			window_manager_stop_running(manager);
		}
		else
		{
			window_manager_process_event(manager, &event);
		}
	}

	window_t* window = manager->impl->window_head;
	while(window)
	{
		window_t* next = window->impl->next;
		window_free(window);
		window = next;
	}

	manager->impl->window_head = NULL;
	manager->impl->window_count = 0;
}
