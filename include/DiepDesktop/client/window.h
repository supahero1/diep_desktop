#pragma once

#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/event.h>
#include <DiepDesktop/client/color.h>
#include <DiepDesktop/client/tex/base.h>

#define CGLM_FORCE_RADIANS
#define CGLM_FORCE_LEFT_HANDED
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#define CGLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <cglm/cglm.h>

#ifdef KEY_EXECUTE
	#undef KEY_EXECUTE
#endif


extern uint64_t
WindowGetTime(
	void
	);


typedef enum WindowCursor
{
	WINDOW_CURSOR_DEFAULT,
	WINDOW_CURSOR_TYPING,
	WINDOW_CURSOR_POINTING,
	kWINDOW_CURSOR
}
WindowCursor;


extern void
WindowSetCursor(
	WindowCursor CursorID
	);


extern void
WindowStartTyping(
	void
	);


extern void
WindowStopTyping(
	void
	);


typedef struct String
{
	char* Text;
	uint32_t Length;
}
String;


extern String
WindowGetClipboard(
	void
	);


extern void
WindowSetClipboard(
	const char* Text
	);


extern void
WindowInit(
	void
	);


extern void
WindowRun(
	void
	);


extern void
WindowFree(
	void
	);


typedef enum MouseButton
{
	MOUSE_BUTTON_UNKNOWN,
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_X1,
	MOUSE_BUTTON_X2,
	kMOUSE_BUTTON
}
MouseButton;


typedef enum Key
{
	KEY_UNKNOWN,
	KEY_RETURN,
	KEY_ESCAPE,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_SPACE,
	KEY_EXCLAIM,
	KEY_DBLAPOSTROPHE,
	KEY_HASH,
	KEY_DOLLAR,
	KEY_PERCENT,
	KEY_AMPERSAND,
	KEY_APOSTROPHE,
	KEY_LEFTPAREN,
	KEY_RIGHTPAREN,
	KEY_ASTERISK,
	KEY_PLUS,
	KEY_COMMA,
	KEY_MINUS,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_COLON,
	KEY_SEMICOLON,
	KEY_LESS,
	KEY_EQUALS,
	KEY_GREATER,
	KEY_QUESTION,
	KEY_AT,
	KEY_LEFTBRACKET,
	KEY_BACKSLASH,
	KEY_RIGHTBRACKET,
	KEY_CARET,
	KEY_UNDERSCORE,
	KEY_GRAVE,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_LEFTBRACE,
	KEY_PIPE,
	KEY_RIGHTBRACE,
	KEY_TILDE,
	KEY_DELETE,
	KEY_PLUSMINUS,
	KEY_CAPSLOCK,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_PRINTSCREEN,
	KEY_SCROLLLOCK,
	KEY_PAUSE,
	KEY_INSERT,
	KEY_HOME,
	KEY_PAGEUP,
	KEY_END,
	KEY_PAGEDOWN,
	KEY_RIGHT,
	KEY_LEFT,
	KEY_DOWN,
	KEY_UP,
	KEY_NUMLOCKCLEAR,
	KEY_KP_DIVIDE,
	KEY_KP_MULTIPLY,
	KEY_KP_MINUS,
	KEY_KP_PLUS,
	KEY_KP_ENTER,
	KEY_KP_1,
	KEY_KP_2,
	KEY_KP_3,
	KEY_KP_4,
	KEY_KP_5,
	KEY_KP_6,
	KEY_KP_7,
	KEY_KP_8,
	KEY_KP_9,
	KEY_KP_0,
	KEY_KP_PERIOD,
	KEY_APPLICATION,
	KEY_POWER,
	KEY_KP_EQUALS,
	KEY_F13,
	KEY_F14,
	KEY_F15,
	KEY_F16,
	KEY_F17,
	KEY_F18,
	KEY_F19,
	KEY_F20,
	KEY_F21,
	KEY_F22,
	KEY_F23,
	KEY_F24,
	KEY_EXECUTE,
	KEY_HELP,
	KEY_MENU,
	KEY_SELECT,
	KEY_STOP,
	KEY_AGAIN,
	KEY_UNDO,
	KEY_CUT,
	KEY_COPY,
	KEY_PASTE,
	KEY_FIND,
	KEY_MUTE,
	KEY_VOLUMEUP,
	KEY_VOLUMEDOWN,
	KEY_KP_COMMA,
	KEY_KP_EQUALSAS400,
	KEY_ALTERASE,
	KEY_SYSREQ,
	KEY_CANCEL,
	KEY_CLEAR,
	KEY_PRIOR,
	KEY_RETURN2,
	KEY_SEPARATOR,
	KEY_OUT,
	KEY_OPER,
	KEY_CLEARAGAIN,
	KEY_CRSEL,
	KEY_EXSEL,
	KEY_KP_00,
	KEY_KP_000,
	KEY_THOUSANDSSEPARATOR,
	KEY_DECIMALSEPARATOR,
	KEY_CURRENCYUNIT,
	KEY_CURRENCYSUBUNIT,
	KEY_KP_LEFTPAREN,
	KEY_KP_RIGHTPAREN,
	KEY_KP_LEFTBRACE,
	KEY_KP_RIGHTBRACE,
	KEY_KP_TAB,
	KEY_KP_BACKSPACE,
	KEY_KP_A,
	KEY_KP_B,
	KEY_KP_C,
	KEY_KP_D,
	KEY_KP_E,
	KEY_KP_F,
	KEY_KP_XOR,
	KEY_KP_POWER,
	KEY_KP_PERCENT,
	KEY_KP_LESS,
	KEY_KP_GREATER,
	KEY_KP_AMPERSAND,
	KEY_KP_DBLAMPERSAND,
	KEY_KP_VERTICALBAR,
	KEY_KP_DBLVERTICALBAR,
	KEY_KP_COLON,
	KEY_KP_HASH,
	KEY_KP_SPACE,
	KEY_KP_AT,
	KEY_KP_EXCLAM,
	KEY_KP_MEMSTORE,
	KEY_KP_MEMRECALL,
	KEY_KP_MEMCLEAR,
	KEY_KP_MEMADD,
	KEY_KP_MEMSUBTRACT,
	KEY_KP_MEMMULTIPLY,
	KEY_KP_MEMDIVIDE,
	KEY_KP_PLUSMINUS,
	KEY_KP_CLEAR,
	KEY_KP_CLEARENTRY,
	KEY_KP_BINARY,
	KEY_KP_OCTAL,
	KEY_KP_DECIMAL,
	KEY_KP_HEXADECIMAL,
	KEY_LCTRL,
	KEY_LSHIFT,
	KEY_LALT,
	KEY_LGUI,
	KEY_RCTRL,
	KEY_RSHIFT,
	KEY_RALT,
	KEY_RGUI,
	KEY_MODE,
	KEY_SLEEP,
	KEY_WAKE,
	KEY_CHANNEL_INCREMENT,
	KEY_CHANNEL_DECREMENT,
	KEY_MEDIA_PLAY,
	KEY_MEDIA_PAUSE,
	KEY_MEDIA_RECORD,
	KEY_MEDIA_FAST_FORWARD,
	KEY_MEDIA_REWIND,
	KEY_MEDIA_NEXT_TRACK,
	KEY_MEDIA_PREVIOUS_TRACK,
	KEY_MEDIA_STOP,
	KEY_MEDIA_EJECT,
	KEY_MEDIA_PLAY_PAUSE,
	KEY_MEDIA_SELECT,
	KEY_AC_NEW,
	KEY_AC_OPEN,
	KEY_AC_CLOSE,
	KEY_AC_EXIT,
	KEY_AC_SAVE,
	KEY_AC_PRINT,
	KEY_AC_PROPERTIES,
	KEY_AC_SEARCH,
	KEY_AC_HOME,
	KEY_AC_BACK,
	KEY_AC_FORWARD,
	KEY_AC_STOP,
	KEY_AC_REFRESH,
	KEY_AC_BOOKMARKS,
	KEY_SOFTLEFT,
	KEY_SOFTRIGHT,
	KEY_CALL,
	KEY_ENDCALL,
	kKEY
}
Key;


typedef enum KeyMod
{
	KEY_MOD_NONE = 0,
	KEY_MOD_SHIFT = 1,
	KEY_MOD_CTRL = 2,
	KEY_MOD_ALT = 4,
	KEY_MOD_GUI = 8,
	KEY_MOD_CAPS_LOCK = 16,
	kKEY_MOD
}
KeyMod;


typedef struct WindowResizeData
{
	Pair OldSize;
	Pair NewSize;
}
WindowResizeData;

extern EventTarget WindowResizeTarget;


typedef struct WindowFocusData
{
	uint8_t _Empty;
}
WindowFocusData;

extern EventTarget WindowFocusTarget;


typedef struct WindowBlurData
{
	uint8_t _Empty;
}
WindowBlurData;

extern EventTarget WindowBlurTarget;


typedef struct WindowKeyDownData
{
	Key Key;
	KeyMod Mods;
	uint8_t Repeat;
}
WindowKeyDownData;

extern EventTarget WindowKeyDownTarget;


typedef struct WindowKeyUpData
{
	Key Key;
	KeyMod Mods;
}
WindowKeyUpData;

extern EventTarget WindowKeyUpTarget;


typedef struct WindowTextData
{
	const char* Text;
	uint32_t Length;
}
WindowTextData;

extern EventTarget WindowTextTarget;


typedef struct WindowMouseDownData
{
	MouseButton Button;
	Pair Position;
	uint8_t Clicks;
}
WindowMouseDownData;

extern EventTarget WindowMouseDownTarget;


typedef struct WindowMouseUpData
{
	MouseButton Button;
	Pair Position;
}
WindowMouseUpData;

extern EventTarget WindowMouseUpTarget;


typedef struct WindowMouseMoveData
{
	Pair OldPosition;
	Pair NewPosition;
}
WindowMouseMoveData;

extern EventTarget WindowMouseMoveTarget;


typedef struct WindowMouseScrollData
{
	float OffsetY;
}
WindowMouseScrollData;

extern EventTarget WindowMouseScrollTarget;


extern Pair
WindowGetSize(
	void
	);


extern Pair
WindowGetMousePosition(
	void
	);


typedef struct VkVertexInstanceInput
{
	vec2 Position;
	vec2 Dimensions;
	ARGB WhiteColor;
	float WhiteDepth;
	ARGB BlackColor;
	float BlackDepth;
	TexInfo Texture;
	float Rotation;
	vec2 TexScale;
	vec2 TexOffset;
}
VkVertexInstanceInput;

typedef VkVertexInstanceInput DrawData;


typedef struct WindowDrawData
{
	uint8_t _Empty;
}
WindowDrawData;

extern EventTarget WindowDrawTarget;


extern void
WindowAddDrawData(
	const DrawData* Data
	);
