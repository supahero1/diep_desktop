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

#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/color.h>
#include <DiepDesktop/shared/event.h>
#include <DiepDesktop/client/tex/base.h>

#define CGLM_FORCE_RADIANS
#define CGLM_FORCE_LEFT_HANDED
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#define CGLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <cglm/cglm.h>


typedef struct window_draw_data
{
	vec2         pos;
	vec2         size;
	float        angle;
	color_argb_t white_color;
	float        white_depth;
	color_argb_t black_color;
	float        black_depth;
	tex_t        tex;
	vec2         tex_scale;
	vec2         tex_offset;
}
window_draw_data_t;


extern void
window_add_draw_data(
	const window_draw_data_t* data
	);


typedef enum window_cursor
{
	WINDOW_CURSOR_DEFAULT,
	WINDOW_CURSOR_TYPING,
	WINDOW_CURSOR_POINTING,
	WINDOW_CURSOR__COUNT
}
window_cursor_t;


extern void
window_set_cursor(
	window_cursor_t cursor
	);


extern void
window_start_typing(
	void
	);


extern void
window_stop_typing(
	void
	);


extern char*
window_get_clipboard(
	uint32_t* len
	);


extern void
window_set_clipboard(
	const char* str
	);


extern pair_t
window_get_size(
	void
	);


extern pair_t
window_get_mouse_pos(
	void
	);


extern void
window_init(
	void
	);


extern void
window_run(
	void
	);


extern void
window_free(
	void
	);


typedef enum window_button
{
	WINDOW_BUTTON_UNKNOWN,
	WINDOW_BUTTON_LEFT,
	WINDOW_BUTTON_MIDDLE,
	WINDOW_BUTTON_RIGHT,
	WINDOW_BUTTON_X1,
	WINDOW_BUTTON_X2,
	WINDOW_BUTTON__COUNT
}
window_button_t;


#define __(x) WINDOW_KEY_##x
#define _(x) __(x),

typedef enum window_key
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
	WINDOW_KEY__COUNT
}
window_key_t;

#undef _
#undef __


typedef enum window_mod
{
	WINDOW_KEY_MOD_NONE = 0,
	WINDOW_KEY_MOD_SHIFT = 1,
	WINDOW_KEY_MOD_CTRL = 2,
	WINDOW_KEY_MOD_ALT = 4,
	WINDOW_KEY_MOD_GUI = 8,
	WINDOW_KEY_MOD_CAPS_LOCK = 16,
	WINDOW_KEY_MOD__COUNT
}
window_mod_t;


typedef struct window_resize_event_data
{
	pair_t old_size;
	pair_t new_size;
}
window_resize_event_data_t;

extern event_target_t window_resize_target;


typedef struct window_focus_event_data
{
	uint8_t _;
}
window_focus_event_data_t;

extern event_target_t window_focus_target;


typedef struct window_blur_event_data
{
	uint8_t _;
}
window_blur_event_data_t;

extern event_target_t window_blur_target;


typedef struct window_key_down_event_data
{
	window_key_t key;
	window_mod_t mods;
	uint8_t repeat;
}
window_key_down_event_data_t;

extern event_target_t window_key_down_target;


typedef struct window_key_up_event_data
{
	window_key_t key;
	window_mod_t mods;
}
window_key_up_event_data_t;

extern event_target_t window_key_up_target;


typedef struct window_text_event_data
{
	const char* str;
	uint32_t len;
}
window_text_event_data_t;

extern event_target_t window_text_target;


typedef struct window_mouse_down_event_data
{
	window_button_t button;
	pair_t pos;
	uint8_t clicks;
}
window_mouse_down_event_data_t;

extern event_target_t window_mouse_down_target;


typedef struct window_mouse_up_event_data
{
	window_button_t button;
	pair_t pos;
}
window_mouse_up_event_data_t;

extern event_target_t window_mouse_up_target;


typedef struct window_mouse_move_event_data
{
	pair_t old_pos;
	pair_t new_pos;
}
window_mouse_move_event_data_t;

extern event_target_t window_mouse_move_target;


typedef struct window_mouse_scroll_event_data
{
	float offset_y;
}
window_mouse_scroll_event_data_t;

extern event_target_t window_mouse_scroll_target;


typedef struct window_draw_event_data
{
	uint8_t _;
}
window_draw_event_data_t;

extern event_target_t window_draw_target;
