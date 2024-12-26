#include <DiepDesktop/client/dds.h>
#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/client/window.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/client/tex/base.h>
#include <DiepDesktop/shared/alloc_ext.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <DiepDesktop/client/volk.h>

#include <zstd.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdatomic.h>

#define VULKAN_MAX_IMAGES 8


Static SDL_PropertiesID WindowProps;
Static SDL_Window* Window;
Static SDL_Cursor* Cursors[kWINDOW_CURSOR];
Static WindowCursor CurrentCursor = WINDOW_CURSOR_DEFAULT;
Static Pair WindowSize;
Static IPair OldWindowPosition;
Static IPair OldWindowSize;
Static Pair MousePosition;
Static bool Fullscreen = false;
Static bool FirstFrame = true;



Static const char* vkInstanceExtensions[] =
{
#ifndef NDEBUG
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

Static const char* vkLayers[] =
{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation"
#endif
};

Static VkInstance vkInstance;


Static VkSurfaceKHR vkSurface;
Static VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;


Static const char* vkDeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


Static uint32_t vkQueueID;
Static VkSampleCountFlagBits vkSamples;
Static VkPhysicalDeviceLimits vkLimits;

Static VkPhysicalDevice vkPhysicalDevice;
Static VkDevice vkDevice;
Static VkQueue vkQueue;
Static VkPhysicalDeviceMemoryProperties vkMemoryProperties;

Static VkCommandPool vkCommandPool;
Static VkCommandBuffer vkCommandBuffer;
Static VkFence vkFence;

Static VkExtent2D vkExtent;
Static uint32_t vkMinImageCount;
Static VkSurfaceTransformFlagBitsKHR vkTransform;
Static VkPresentModeKHR vkPresentMode;


Static VkSwapchainKHR vkSwapchain;
Static uint32_t vkImageCount;

typedef struct VkFrame
{
	VkImage Image;
	VkImageView ImageView;
	VkFramebuffer Framebuffer;
	VkCommandBuffer CommandBuffer;
}
VkFrame;

Static VkFrame vkFrames[VULKAN_MAX_IMAGES];


typedef enum Semaphore
{
	SEMAPHORE_IMAGE_AVAILABLE,
	SEMAPHORE_RENDER_FINISHED,
	kSEMAPHORE
}
Semaphore;

typedef enum Fence
{
	FENCE_IN_FLIGHT,
	kFENCE
}
Fence;

typedef struct VkBarrier
{
	VkSemaphore Semaphores[kSEMAPHORE];
	VkFence Fences[kFENCE];
}
VkBarrier;

Static VkBarrier vkBarriers[VULKAN_MAX_IMAGES];
Static VkBarrier* vkBarrier = vkBarriers;


Static VkViewport vkViewport;
Static VkRect2D vkScissor;


Static VkBuffer vkCopyBuffer;
Static VkDeviceMemory vkCopyBufferMemory;


Static VkSampler vkSampler;


typedef enum ImageType
{
	IMAGE_TYPE_DEPTH_STENCIL,
	IMAGE_TYPE_MULTISAMPLED,
	IMAGE_TYPE_TEXTURE
}
ImageType;

typedef struct Image
{
	const char* Path;

	uint32_t Width;
	uint32_t Height;
	uint32_t Layers;

	VkFormat Format;
	ImageType Type;

	VkImage Image;
	VkImageView View;
	VkDeviceMemory Memory;

	VkImageAspectFlags Aspect;
	VkImageUsageFlags Usage;
	VkSampleCountFlagBits Samples;
}
Image;

Static Image vkDepthBuffer =
{
	.Format = VK_FORMAT_D32_SFLOAT,
	.Type = IMAGE_TYPE_DEPTH_STENCIL
};

Static Image vkMultisampling =
{
	.Format = VK_FORMAT_B8G8R8A8_SRGB,
	.Type = IMAGE_TYPE_MULTISAMPLED
};

Static Image vkTextures[TEXTURES_NUM];


typedef struct VkVertexVertexInput
{
	vec2 Position;
	vec2 TexCoord;
}
VkVertexVertexInput;

Static const VkVertexVertexInput vkVertexVertexInput[] =
{
	{ { -0.5f, -0.5f }, { 0.0f, 0.0f } },
	{ {  0.5f, -0.5f }, { 1.0f, 0.0f } },
	{ { -0.5f,  0.5f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f }, { 1.0f, 1.0f } },
};

Static VkBuffer vkVertexVertexInputBuffer;
Static VkDeviceMemory vkVertexVertexInputMemory;

Static VkBuffer vkVertexInstanceInputBuffer;
Static VkDeviceMemory vkVertexInstanceInputMemory;

Static VkBuffer vkDrawCountBuffer;
Static VkDeviceMemory vkDrawCountMemory;


typedef struct VkVertexConstantInput
{
	Pair WindowSize;
}
VkVertexConstantInput;

Static VkVertexConstantInput vkConstants;


Static VkDescriptorSetLayout vkDescriptors;
Static VkRenderPass vkRenderPass;
Static VkPipelineLayout vkPipelineLayout;
Static VkPipeline vkPipeline;
Static VkDescriptorPool vkDescriptorPool;
Static VkDescriptorSet vkDescriptorSet;


Static ThreadID vkThread;
Static _Atomic uint8_t vkShouldRun;
Static Mutex Mtx;
Static CondVar Cond;
Static bool Resized;
Static Mutex WindowMtx;


EventTarget WindowResizeTarget;
EventTarget WindowFocusTarget;
EventTarget WindowBlurTarget;
EventTarget WindowKeyDownTarget;
EventTarget WindowKeyUpTarget;
EventTarget WindowTextTarget;
EventTarget WindowMouseDownTarget;
EventTarget WindowMouseUpTarget;
EventTarget WindowMouseMoveTarget;
EventTarget WindowMouseScrollTarget;


Static DrawData* DrawDataBuffer;
Static uint32_t DrawDataCount;

EventTarget WindowDrawTarget;



Pair
WindowGetSize(
	void
	)
{
	return WindowSize;
}


Pair
WindowGetMousePosition(
	void
	)
{
	return MousePosition;
}


#ifndef NDEBUG

Static VkDebugUtilsMessengerEXT vkDebugMessenger;

Static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
	VkDebugUtilsMessageTypeFlagsEXT Type,
	const VkDebugUtilsMessengerCallbackDataEXT* Data,
	void* UserData
	)
{
	puts(Data->pMessage);

	// if(Severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	// {
	// 	abort();
	// }

	return VK_FALSE;
}

#endif /* NDEBUG */


Static void
SDLError(
	void
	)
{
	const char* Error = SDL_GetError();
	if(Error)
	{
		puts(Error);
	}
}


Static void
VulkanToggleFullscreen(
	void
	)
{
	if(Fullscreen)
	{
		bool Status = SDL_SetWindowFullscreen(Window, false);
		AssertTrue(Status);

		Status = SDL_SetWindowSize(Window, OldWindowSize.W, OldWindowSize.H);
		AssertTrue(Status);

		Status = SDL_SetWindowPosition(Window, OldWindowPosition.X, OldWindowPosition.Y);
		AssertTrue(Status);
	}
	else
	{
		bool Status = SDL_GetWindowSize(Window, &OldWindowSize.W, &OldWindowSize.H);
		AssertTrue(Status);

		Status = SDL_GetWindowPosition(Window, &OldWindowPosition.X, &OldWindowPosition.Y);
		AssertTrue(Status);

		Status = SDL_SetWindowFullscreen(Window, true);
		AssertTrue(Status);
	}

	Fullscreen = !Fullscreen;
}


uint64_t
WindowGetTime(
	void
	)
{
	return SDL_GetTicksNS();
}


void
WindowSetCursor(
	WindowCursor CursorID
	)
{
	if(CursorID != CurrentCursor)
	{
		CurrentCursor = CursorID;
		SDL_SetCursor(Cursors[CursorID]);
	}
}


String
WindowGetClipboard(
	void
	)
{
	char* Text = SDL_GetClipboardText();
	if(!Text)
	{
		return (String){0};
	}

	uint32_t Length = strlen(Text) + 1;

	char* NewText = AllocMalloc(Length);
	if(!NewText)
	{
		SDL_free(Text);
		return (String){0};
	}

	memcpy(NewText, Text, Length);

	SDL_free(Text);

	return (String){ .Text = NewText, .Length = Length };
}


void
WindowSetClipboard(
	const char* Text
	)
{
	SDL_SetClipboardText(Text);
}


Static void
WindowQuit(
	void
	)
{
	SDL_PushEvent(&(SDL_Event){ .type = SDL_EVENT_QUIT });
}


Static Key
MapSDLKey(
	int Key
	)
{
	switch(Key)
	{

	case SDLK_UNKNOWN: return KEY_UNKNOWN;
	case SDLK_RETURN: return KEY_RETURN;
	case SDLK_ESCAPE: return KEY_ESCAPE;
	case SDLK_BACKSPACE: return KEY_BACKSPACE;
	case SDLK_TAB: return KEY_TAB;
	case SDLK_SPACE: return KEY_SPACE;
	case SDLK_EXCLAIM: return KEY_EXCLAIM;
	case SDLK_DBLAPOSTROPHE: return KEY_DBLAPOSTROPHE;
	case SDLK_HASH: return KEY_HASH;
	case SDLK_DOLLAR: return KEY_DOLLAR;
	case SDLK_PERCENT: return KEY_PERCENT;
	case SDLK_AMPERSAND: return KEY_AMPERSAND;
	case SDLK_APOSTROPHE: return KEY_APOSTROPHE;
	case SDLK_LEFTPAREN: return KEY_LEFTPAREN;
	case SDLK_RIGHTPAREN: return KEY_RIGHTPAREN;
	case SDLK_ASTERISK: return KEY_ASTERISK;
	case SDLK_PLUS: return KEY_PLUS;
	case SDLK_COMMA: return KEY_COMMA;
	case SDLK_MINUS: return KEY_MINUS;
	case SDLK_PERIOD: return KEY_PERIOD;
	case SDLK_SLASH: return KEY_SLASH;
	case SDLK_0: return KEY_0;
	case SDLK_1: return KEY_1;
	case SDLK_2: return KEY_2;
	case SDLK_3: return KEY_3;
	case SDLK_4: return KEY_4;
	case SDLK_5: return KEY_5;
	case SDLK_6: return KEY_6;
	case SDLK_7: return KEY_7;
	case SDLK_8: return KEY_8;
	case SDLK_9: return KEY_9;
	case SDLK_COLON: return KEY_COLON;
	case SDLK_SEMICOLON: return KEY_SEMICOLON;
	case SDLK_LESS: return KEY_LESS;
	case SDLK_EQUALS: return KEY_EQUALS;
	case SDLK_GREATER: return KEY_GREATER;
	case SDLK_QUESTION: return KEY_QUESTION;
	case SDLK_AT: return KEY_AT;
	case SDLK_LEFTBRACKET: return KEY_LEFTBRACKET;
	case SDLK_BACKSLASH: return KEY_BACKSLASH;
	case SDLK_RIGHTBRACKET: return KEY_RIGHTBRACKET;
	case SDLK_CARET: return KEY_CARET;
	case SDLK_UNDERSCORE: return KEY_UNDERSCORE;
	case SDLK_GRAVE: return KEY_GRAVE;
	case SDLK_A: return KEY_A;
	case SDLK_B: return KEY_B;
	case SDLK_C: return KEY_C;
	case SDLK_D: return KEY_D;
	case SDLK_E: return KEY_E;
	case SDLK_F: return KEY_F;
	case SDLK_G: return KEY_G;
	case SDLK_H: return KEY_H;
	case SDLK_I: return KEY_I;
	case SDLK_J: return KEY_J;
	case SDLK_K: return KEY_K;
	case SDLK_L: return KEY_L;
	case SDLK_M: return KEY_M;
	case SDLK_N: return KEY_N;
	case SDLK_O: return KEY_O;
	case SDLK_P: return KEY_P;
	case SDLK_Q: return KEY_Q;
	case SDLK_R: return KEY_R;
	case SDLK_S: return KEY_S;
	case SDLK_T: return KEY_T;
	case SDLK_U: return KEY_U;
	case SDLK_V: return KEY_V;
	case SDLK_W: return KEY_W;
	case SDLK_X: return KEY_X;
	case SDLK_Y: return KEY_Y;
	case SDLK_Z: return KEY_Z;
	case SDLK_LEFTBRACE: return KEY_LEFTBRACE;
	case SDLK_PIPE: return KEY_PIPE;
	case SDLK_RIGHTBRACE: return KEY_RIGHTBRACE;
	case SDLK_TILDE: return KEY_TILDE;
	case SDLK_DELETE: return KEY_DELETE;
	case SDLK_PLUSMINUS: return KEY_PLUSMINUS;
	case SDLK_CAPSLOCK: return KEY_CAPSLOCK;
	case SDLK_F1: return KEY_F1;
	case SDLK_F2: return KEY_F2;
	case SDLK_F3: return KEY_F3;
	case SDLK_F4: return KEY_F4;
	case SDLK_F5: return KEY_F5;
	case SDLK_F6: return KEY_F6;
	case SDLK_F7: return KEY_F7;
	case SDLK_F8: return KEY_F8;
	case SDLK_F9: return KEY_F9;
	case SDLK_F10: return KEY_F10;
	case SDLK_F11: return KEY_F11;
	case SDLK_F12: return KEY_F12;
	case SDLK_PRINTSCREEN: return KEY_PRINTSCREEN;
	case SDLK_SCROLLLOCK: return KEY_SCROLLLOCK;
	case SDLK_PAUSE: return KEY_PAUSE;
	case SDLK_INSERT: return KEY_INSERT;
	case SDLK_HOME: return KEY_HOME;
	case SDLK_PAGEUP: return KEY_PAGEUP;
	case SDLK_END: return KEY_END;
	case SDLK_PAGEDOWN: return KEY_PAGEDOWN;
	case SDLK_RIGHT: return KEY_RIGHT;
	case SDLK_LEFT: return KEY_LEFT;
	case SDLK_DOWN: return KEY_DOWN;
	case SDLK_UP: return KEY_UP;
	case SDLK_NUMLOCKCLEAR: return KEY_NUMLOCKCLEAR;
	case SDLK_KP_DIVIDE: return KEY_KP_DIVIDE;
	case SDLK_KP_MULTIPLY: return KEY_KP_MULTIPLY;
	case SDLK_KP_MINUS: return KEY_KP_MINUS;
	case SDLK_KP_PLUS: return KEY_KP_PLUS;
	case SDLK_KP_ENTER: return KEY_KP_ENTER;
	case SDLK_KP_1: return KEY_KP_1;
	case SDLK_KP_2: return KEY_KP_2;
	case SDLK_KP_3: return KEY_KP_3;
	case SDLK_KP_4: return KEY_KP_4;
	case SDLK_KP_5: return KEY_KP_5;
	case SDLK_KP_6: return KEY_KP_6;
	case SDLK_KP_7: return KEY_KP_7;
	case SDLK_KP_8: return KEY_KP_8;
	case SDLK_KP_9: return KEY_KP_9;
	case SDLK_KP_0: return KEY_KP_0;
	case SDLK_KP_PERIOD: return KEY_KP_PERIOD;
	case SDLK_APPLICATION: return KEY_APPLICATION;
	case SDLK_POWER: return KEY_POWER;
	case SDLK_KP_EQUALS: return KEY_KP_EQUALS;
	case SDLK_F13: return KEY_F13;
	case SDLK_F14: return KEY_F14;
	case SDLK_F15: return KEY_F15;
	case SDLK_F16: return KEY_F16;
	case SDLK_F17: return KEY_F17;
	case SDLK_F18: return KEY_F18;
	case SDLK_F19: return KEY_F19;
	case SDLK_F20: return KEY_F20;
	case SDLK_F21: return KEY_F21;
	case SDLK_F22: return KEY_F22;
	case SDLK_F23: return KEY_F23;
	case SDLK_F24: return KEY_F24;
	case SDLK_EXECUTE: return KEY_EXECUTE;
	case SDLK_HELP: return KEY_HELP;
	case SDLK_MENU: return KEY_MENU;
	case SDLK_SELECT: return KEY_SELECT;
	case SDLK_STOP: return KEY_STOP;
	case SDLK_AGAIN: return KEY_AGAIN;
	case SDLK_UNDO: return KEY_UNDO;
	case SDLK_CUT: return KEY_CUT;
	case SDLK_COPY: return KEY_COPY;
	case SDLK_PASTE: return KEY_PASTE;
	case SDLK_FIND: return KEY_FIND;
	case SDLK_MUTE: return KEY_MUTE;
	case SDLK_VOLUMEUP: return KEY_VOLUMEUP;
	case SDLK_VOLUMEDOWN: return KEY_VOLUMEDOWN;
	case SDLK_KP_COMMA: return KEY_KP_COMMA;
	case SDLK_KP_EQUALSAS400: return KEY_KP_EQUALSAS400;
	case SDLK_ALTERASE: return KEY_ALTERASE;
	case SDLK_SYSREQ: return KEY_SYSREQ;
	case SDLK_CANCEL: return KEY_CANCEL;
	case SDLK_CLEAR: return KEY_CLEAR;
	case SDLK_PRIOR: return KEY_PRIOR;
	case SDLK_RETURN2: return KEY_RETURN2;
	case SDLK_SEPARATOR: return KEY_SEPARATOR;
	case SDLK_OUT: return KEY_OUT;
	case SDLK_OPER: return KEY_OPER;
	case SDLK_CLEARAGAIN: return KEY_CLEARAGAIN;
	case SDLK_CRSEL: return KEY_CRSEL;
	case SDLK_EXSEL: return KEY_EXSEL;
	case SDLK_KP_00: return KEY_KP_00;
	case SDLK_KP_000: return KEY_KP_000;
	case SDLK_THOUSANDSSEPARATOR: return KEY_THOUSANDSSEPARATOR;
	case SDLK_DECIMALSEPARATOR: return KEY_DECIMALSEPARATOR;
	case SDLK_CURRENCYUNIT: return KEY_CURRENCYUNIT;
	case SDLK_CURRENCYSUBUNIT: return KEY_CURRENCYSUBUNIT;
	case SDLK_KP_LEFTPAREN: return KEY_KP_LEFTPAREN;
	case SDLK_KP_RIGHTPAREN: return KEY_KP_RIGHTPAREN;
	case SDLK_KP_LEFTBRACE: return KEY_KP_LEFTBRACE;
	case SDLK_KP_RIGHTBRACE: return KEY_KP_RIGHTBRACE;
	case SDLK_KP_TAB: return KEY_KP_TAB;
	case SDLK_KP_BACKSPACE: return KEY_KP_BACKSPACE;
	case SDLK_KP_A: return KEY_KP_A;
	case SDLK_KP_B: return KEY_KP_B;
	case SDLK_KP_C: return KEY_KP_C;
	case SDLK_KP_D: return KEY_KP_D;
	case SDLK_KP_E: return KEY_KP_E;
	case SDLK_KP_F: return KEY_KP_F;
	case SDLK_KP_XOR: return KEY_KP_XOR;
	case SDLK_KP_POWER: return KEY_KP_POWER;
	case SDLK_KP_PERCENT: return KEY_KP_PERCENT;
	case SDLK_KP_LESS: return KEY_KP_LESS;
	case SDLK_KP_GREATER: return KEY_KP_GREATER;
	case SDLK_KP_AMPERSAND: return KEY_KP_AMPERSAND;
	case SDLK_KP_DBLAMPERSAND: return KEY_KP_DBLAMPERSAND;
	case SDLK_KP_VERTICALBAR: return KEY_KP_VERTICALBAR;
	case SDLK_KP_DBLVERTICALBAR: return KEY_KP_DBLVERTICALBAR;
	case SDLK_KP_COLON: return KEY_KP_COLON;
	case SDLK_KP_HASH: return KEY_KP_HASH;
	case SDLK_KP_SPACE: return KEY_KP_SPACE;
	case SDLK_KP_AT: return KEY_KP_AT;
	case SDLK_KP_EXCLAM: return KEY_KP_EXCLAM;
	case SDLK_KP_MEMSTORE: return KEY_KP_MEMSTORE;
	case SDLK_KP_MEMRECALL: return KEY_KP_MEMRECALL;
	case SDLK_KP_MEMCLEAR: return KEY_KP_MEMCLEAR;
	case SDLK_KP_MEMADD: return KEY_KP_MEMADD;
	case SDLK_KP_MEMSUBTRACT: return KEY_KP_MEMSUBTRACT;
	case SDLK_KP_MEMMULTIPLY: return KEY_KP_MEMMULTIPLY;
	case SDLK_KP_MEMDIVIDE: return KEY_KP_MEMDIVIDE;
	case SDLK_KP_PLUSMINUS: return KEY_KP_PLUSMINUS;
	case SDLK_KP_CLEAR: return KEY_KP_CLEAR;
	case SDLK_KP_CLEARENTRY: return KEY_KP_CLEARENTRY;
	case SDLK_KP_BINARY: return KEY_KP_BINARY;
	case SDLK_KP_OCTAL: return KEY_KP_OCTAL;
	case SDLK_KP_DECIMAL: return KEY_KP_DECIMAL;
	case SDLK_KP_HEXADECIMAL: return KEY_KP_HEXADECIMAL;
	case SDLK_LCTRL: return KEY_LCTRL;
	case SDLK_LSHIFT: return KEY_LSHIFT;
	case SDLK_LALT: return KEY_LALT;
	case SDLK_LGUI: return KEY_LGUI;
	case SDLK_RCTRL: return KEY_RCTRL;
	case SDLK_RSHIFT: return KEY_RSHIFT;
	case SDLK_RALT: return KEY_RALT;
	case SDLK_RGUI: return KEY_RGUI;
	case SDLK_MODE: return KEY_MODE;
	case SDLK_SLEEP: return KEY_SLEEP;
	case SDLK_WAKE: return KEY_WAKE;
	case SDLK_CHANNEL_INCREMENT: return KEY_CHANNEL_INCREMENT;
	case SDLK_CHANNEL_DECREMENT: return KEY_CHANNEL_DECREMENT;
	case SDLK_MEDIA_PLAY: return KEY_MEDIA_PLAY;
	case SDLK_MEDIA_PAUSE: return KEY_MEDIA_PAUSE;
	case SDLK_MEDIA_RECORD: return KEY_MEDIA_RECORD;
	case SDLK_MEDIA_FAST_FORWARD: return KEY_MEDIA_FAST_FORWARD;
	case SDLK_MEDIA_REWIND: return KEY_MEDIA_REWIND;
	case SDLK_MEDIA_NEXT_TRACK: return KEY_MEDIA_NEXT_TRACK;
	case SDLK_MEDIA_PREVIOUS_TRACK: return KEY_MEDIA_PREVIOUS_TRACK;
	case SDLK_MEDIA_STOP: return KEY_MEDIA_STOP;
	case SDLK_MEDIA_EJECT: return KEY_MEDIA_EJECT;
	case SDLK_MEDIA_PLAY_PAUSE: return KEY_MEDIA_PLAY_PAUSE;
	case SDLK_MEDIA_SELECT: return KEY_MEDIA_SELECT;
	case SDLK_AC_NEW: return KEY_AC_NEW;
	case SDLK_AC_OPEN: return KEY_AC_OPEN;
	case SDLK_AC_CLOSE: return KEY_AC_CLOSE;
	case SDLK_AC_EXIT: return KEY_AC_EXIT;
	case SDLK_AC_SAVE: return KEY_AC_SAVE;
	case SDLK_AC_PRINT: return KEY_AC_PRINT;
	case SDLK_AC_PROPERTIES: return KEY_AC_PROPERTIES;
	case SDLK_AC_SEARCH: return KEY_AC_SEARCH;
	case SDLK_AC_HOME: return KEY_AC_HOME;
	case SDLK_AC_BACK: return KEY_AC_BACK;
	case SDLK_AC_FORWARD: return KEY_AC_FORWARD;
	case SDLK_AC_STOP: return KEY_AC_STOP;
	case SDLK_AC_REFRESH: return KEY_AC_REFRESH;
	case SDLK_AC_BOOKMARKS: return KEY_AC_BOOKMARKS;
	case SDLK_SOFTLEFT: return KEY_SOFTLEFT;
	case SDLK_SOFTRIGHT: return KEY_SOFTRIGHT;
	case SDLK_CALL: return KEY_CALL;
	case SDLK_ENDCALL: return KEY_ENDCALL;
	default: return KEY_UNKNOWN;

	}
}


Static KeyMod
MapSDLMod(
	int Mod
	)
{
	KeyMod Result = 0;

	if(Mod & SDL_KMOD_SHIFT) Result |= KEY_MOD_SHIFT;
	if(Mod & SDL_KMOD_CTRL) Result |= KEY_MOD_CTRL;
	if(Mod & SDL_KMOD_ALT) Result |= KEY_MOD_ALT;
	if(Mod & SDL_KMOD_GUI) Result |= KEY_MOD_GUI;
	if(Mod & SDL_KMOD_CAPS) Result |= KEY_MOD_CAPS_LOCK;

	return Result;
}


Static MouseButton
MapSDLButton(
	int Button
	)
{
	switch(Button)
	{

	case SDL_BUTTON_LEFT: return MOUSE_BUTTON_LEFT;
	case SDL_BUTTON_MIDDLE: return MOUSE_BUTTON_MIDDLE;
	case SDL_BUTTON_RIGHT: return MOUSE_BUTTON_RIGHT;
	case SDL_BUTTON_X1: return MOUSE_BUTTON_X1;
	case SDL_BUTTON_X2: return MOUSE_BUTTON_X2;
	default: return MOUSE_BUTTON_UNKNOWN;

	}
}


Static void
WindowOnEvent(
	SDL_Event* Event
	)
{
	switch(Event->type)
	{

	case SDL_EVENT_WINDOW_RESIZED:
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	case SDL_EVENT_WINDOW_FOCUS_LOST:
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
	case SDL_EVENT_TEXT_INPUT:
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
	case SDL_EVENT_MOUSE_MOTION:
	case SDL_EVENT_MOUSE_WHEEL: break;

	default: return;

	}


	MutexLock(&WindowMtx);


	switch(Event->type)
	{

	case SDL_EVENT_WINDOW_RESIZED:
	{
		if(!Resized)
		{
			Resized = true;
			CondVarWake(&Cond);
		}

		break;
	}

	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	{
		EventNotify(&WindowFocusTarget, &((WindowFocusData){0}));
		break;
	}

	case SDL_EVENT_WINDOW_FOCUS_LOST:
	{
		EventNotify(&WindowBlurTarget, &((WindowBlurData){0}));
		break;
	}

	case SDL_EVENT_KEY_DOWN:
	{
		SDL_KeyboardEvent Key = Event->key;

		if((Key.mod & SDL_KMOD_CTRL) && (Key.key == SDLK_W || Key.key == SDLK_R))
		{
			WindowQuit();
		}

		if(Key.key == SDLK_F11 && !Key.repeat)
		{
			VulkanToggleFullscreen();
		}

		WindowKeyDownData EventData =
		(WindowKeyDownData)
		{
			.Key = MapSDLKey(Key.key),
			.Mods = MapSDLMod(Key.mod),
			.Repeat = Key.repeat
		};

		EventNotify(&WindowKeyDownTarget, &EventData);

		break;
	}

	case SDL_EVENT_KEY_UP:
	{
		SDL_KeyboardEvent Key = Event->key;

		WindowKeyUpData EventData =
		(WindowKeyUpData)
		{
			.Key = MapSDLKey(Key.key),
			.Mods = MapSDLMod(Key.mod)
		};

		EventNotify(&WindowKeyUpTarget, &EventData);

		break;
	}

	case SDL_EVENT_TEXT_INPUT:
	{
		SDL_TextInputEvent Text = Event->text;

		if(!Text.text)
		{
			break;
		}

		WindowTextData EventData =
		(WindowTextData)
		{
			.Text = Text.text,
			.Length = strlen(Text.text)
		};

		EventNotify(&WindowTextTarget, &EventData);

		break;
	}

	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	{
		SDL_MouseButtonEvent Button = Event->button;

		float HalfWidth  = WindowSize.W * 0.5f;
		float HalfHeight = WindowSize.H * 0.5f;

		Pair EventMousePosition =
		(Pair)
		{
			.X = CLAMP_SYM(Button.x - HalfWidth , HalfWidth ),
			.Y = CLAMP_SYM(Button.y - HalfHeight, HalfHeight)
		};

		AssertLT(fabsf(EventMousePosition.X - MousePosition.X), 0.01f);
		AssertLT(fabsf(EventMousePosition.Y - MousePosition.Y), 0.01f);

		WindowMouseDownData EventData =
		(WindowMouseDownData)
		{
			.Button = MapSDLButton(Button.button),
			.Position = MousePosition,
			.Clicks = Button.clicks
		};

		EventNotify(&WindowMouseDownTarget, &EventData);

		break;
	}

	case SDL_EVENT_MOUSE_BUTTON_UP:
	{
		SDL_MouseButtonEvent Button = Event->button;

		float HalfWidth  = WindowSize.W * 0.5f;
		float HalfHeight = WindowSize.H * 0.5f;

		Pair EventMousePosition =
		(Pair)
		{
			.X = CLAMP_SYM(Button.x - HalfWidth , HalfWidth ),
			.Y = CLAMP_SYM(Button.y - HalfHeight, HalfHeight)
		};

		AssertLT(fabsf(EventMousePosition.X - MousePosition.X), 0.01f);
		AssertLT(fabsf(EventMousePosition.Y - MousePosition.Y), 0.01f);

		WindowMouseUpData EventData =
		(WindowMouseUpData)
		{
			.Button = MapSDLButton(Button.button),
			.Position = MousePosition
		};

		EventNotify(&WindowMouseUpTarget, &EventData);

		break;
	}

	case SDL_EVENT_MOUSE_MOTION:
	{
		SDL_MouseMotionEvent Motion = Event->motion;

		WindowMouseMoveData EventData =
		(WindowMouseMoveData)
		{
			.OldPosition = MousePosition
		};

		float HalfWidth  = WindowSize.W * 0.5f;
		float HalfHeight = WindowSize.H * 0.5f;

		MousePosition =
		(Pair)
		{
			.X = CLAMP_SYM(Motion.x - HalfWidth , HalfWidth ),
			.Y = CLAMP_SYM(Motion.y - HalfHeight, HalfHeight)
		};

		EventData.NewPosition = MousePosition;

		EventNotify(&WindowMouseMoveTarget, &EventData);

		break;
	}

	case SDL_EVENT_MOUSE_WHEEL:
	{
		SDL_MouseWheelEvent Wheel = Event->wheel;

		WindowMouseScrollData EventData =
		(WindowMouseScrollData)
		{
			.OffsetY = Event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -Wheel.y : Wheel.y
		};

		EventNotify(&WindowMouseScrollTarget, &EventData);

		break;
	}

	default: break;

	}


	MutexUnlock(&WindowMtx);
}


void
WindowStartTyping(
	void
	)
{
	SDL_StartTextInput(Window);
}


void
WindowStopTyping(
	void
	)
{
	SDL_StopTextInput(Window);
}


Static void
VulkanInitSDL(
	void
	)
{
	bool Status = SDL_InitSubSystem(SDL_INIT_VIDEO);
	HardenedAssertTrue(Status);

	WindowProps = SDL_CreateProperties();
	AssertNEQ(WindowProps, 0);

	Status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
	AssertTrue(Status);

	Status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
	AssertTrue(Status);

	Status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	AssertTrue(Status);

	Status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
	AssertTrue(Status);

	Status = SDL_SetNumberProperty(WindowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, GAME_CONST_DEFAULT_WINDOW_WIDTH);
	AssertTrue(Status);

	Status = SDL_SetNumberProperty(WindowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, GAME_CONST_DEFAULT_WINDOW_HEIGHT);
	AssertTrue(Status);

	Status = SDL_SetStringProperty(WindowProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Game");
	AssertTrue(Status);

	Status = SDL_SetBooleanProperty(WindowProps, SDL_HINT_FORCE_RAISEWINDOW, true);
	AssertTrue(Status);

	Window = SDL_CreateWindowWithProperties(WindowProps);
	if(!Window)
	{
		SDLError();
		HardenedAssertUnreachable();
	}

	Status = SDL_SetWindowMinimumSize(Window, 480, 270);
	AssertTrue(Status);

	// SDL_Rect MouseBound = { 0, 0, 1280, 720 };
	// SDL_SetWindowMouseRect(Window, &MouseBound);

	Cursors[WINDOW_CURSOR_DEFAULT] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	Cursors[WINDOW_CURSOR_TYPING] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
	Cursors[WINDOW_CURSOR_POINTING] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
}


Static void
VulkanDestroySDL(
	void
	)
{
	SDL_Cursor** Cursor = Cursors;
	SDL_Cursor** CursorEnd = Cursor + ARRAYLEN(Cursors);

	do
	{
		SDL_DestroyCursor(*Cursor);
	}
	while(++Cursor != CursorEnd);

	SDL_DestroyWindow(Window);
	SDL_DestroyProperties(WindowProps);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}


Static void
VulkanInitInstance(
	void
	)
{
	volkInitializeCustom((PFN_vkGetInstanceProcAddr) SDL_Vulkan_GetVkGetInstanceProcAddr());

	VkApplicationInfo AppInfo = {0};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pNext = NULL;
	AppInfo.pApplicationName = NULL;
	AppInfo.applicationVersion = 0;
	AppInfo.pEngineName = NULL;
	AppInfo.engineVersion = 0;
	AppInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo InstanceInfo = {0};
	InstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceInfo.flags = 0;
	InstanceInfo.pApplicationInfo = &AppInfo;
	InstanceInfo.enabledLayerCount = ARRAYLEN(vkLayers);
	InstanceInfo.ppEnabledLayerNames = vkLayers;

	uint32_t SDLExtensionCount;
	const char* const* SDLExtensions = SDL_Vulkan_GetInstanceExtensions(&SDLExtensionCount);

	uint32_t ExtensionCount = SDLExtensionCount + ARRAYLEN(vkInstanceExtensions);
	const char* Extensions[ExtensionCount];

	const char** Extension = Extensions;
	for(uint32_t i = 0; i < SDLExtensionCount; ++i)
	{
		*(Extension++) = SDLExtensions[i];
	}
	for(uint32_t i = 0; i < ARRAYLEN(vkInstanceExtensions); ++i)
	{
		*(Extension++) = vkInstanceExtensions[i];
	}

	InstanceInfo.enabledExtensionCount = ExtensionCount;
	InstanceInfo.ppEnabledExtensionNames = Extensions;

#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT DebugInfo = {0};
	DebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	DebugInfo.pNext = NULL;
	DebugInfo.flags = 0;
	DebugInfo.messageSeverity =
		// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	DebugInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	DebugInfo.pfnUserCallback = VulkanDebugCallback;
	DebugInfo.pUserData = NULL;

	InstanceInfo.pNext = &DebugInfo;
#else
	InstanceInfo.pNext = NULL;
#endif

	VkResult Result = vkCreateInstance(&InstanceInfo, NULL, &vkInstance);
	HardenedAssertEQ(Result, VK_SUCCESS);

	volkLoadInstance(vkInstance);

#ifndef NDEBUG
	Result = vkCreateDebugUtilsMessengerEXT(vkInstance, &DebugInfo, NULL, &vkDebugMessenger);
	AssertEQ(Result, VK_SUCCESS);
#endif
}


Static void
VulkanDestroyInstance(
	void
	)
{
#ifndef NDEBUG
	vkDestroyDebugUtilsMessengerEXT(vkInstance, vkDebugMessenger, NULL);
#endif

	vkDestroyInstance(vkInstance, NULL);

	volkFinalize();
}


Static void
VulkanInitSurface(
	void
	)
{
	bool Status = SDL_Vulkan_CreateSurface(Window, vkInstance, NULL, &vkSurface);
	if(Status == false)
	{
		SDLError();
		HardenedAssertUnreachable();
	}
}


Static void
VulkanDestroySurface(
	void
	)
{
	vkDestroySurfaceKHR(vkInstance, vkSurface, NULL);
}


Static void
VulkanBeginCommandBuffer(
	void
	)
{
	VkResult Result = vkWaitForFences(vkDevice, 1, &vkFence, VK_TRUE, UINT64_MAX);
	HardenedAssertEQ(Result, VK_SUCCESS);

	Result = vkResetFences(vkDevice, 1, &vkFence);
	HardenedAssertEQ(Result, VK_SUCCESS);

	Result = vkResetCommandBuffer(vkCommandBuffer, 0);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkCommandBufferBeginInfo BeginInfo = {0};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.pNext = NULL;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	BeginInfo.pInheritanceInfo = NULL;

	Result = vkBeginCommandBuffer(vkCommandBuffer, &BeginInfo);
	HardenedAssertEQ(Result, VK_SUCCESS);
}


Static void
VulkanEndCommandBuffer(
	void
	)
{
	VkResult Result = vkEndCommandBuffer(vkCommandBuffer);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkSubmitInfo SubmitInfo = {0};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pNext = NULL;
	SubmitInfo.waitSemaphoreCount = 0;
	SubmitInfo.pWaitSemaphores = NULL;
	SubmitInfo.pWaitDstStageMask = NULL;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &vkCommandBuffer;
	SubmitInfo.signalSemaphoreCount = 0;
	SubmitInfo.pSignalSemaphores = NULL;

	vkQueueSubmit(vkQueue, 1, &SubmitInfo, vkFence);
}


typedef struct VkDeviceScore
{
	uint32_t Score;
	uint32_t QueueID;
	VkSampleCountFlagBits Samples;
	VkPhysicalDeviceLimits Limits;
}
VkDeviceScore;


Static bool
VulkanGetDeviceFeatures(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	VkPhysicalDeviceFeatures Features;
	vkGetPhysicalDeviceFeatures(Device, &Features);

	if(!Features.sampleRateShading)
	{
		HardenedLogLocation();
		return false;
	}

	if(!Features.textureCompressionBC)
	{
		HardenedLogLocation();
		return false;
	}

	return true;
}


Static bool
VulkanGetDeviceQueues(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	uint32_t QueueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueCount, NULL);

	if(QueueCount == 0)
	{
		HardenedLogLocation();
		return false;
	}

	VkQueueFamilyProperties Queues[QueueCount];
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueCount, Queues);

	VkQueueFamilyProperties* Queue = Queues;

	for(uint32_t i = 0; i < QueueCount; ++i, ++Queue)
	{
		VkBool32 Present;
		VkResult Result = vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, vkSurface, &Present);

		if(Result != VK_SUCCESS)
		{
			HardenedLogLocation();
			return false;
		}

		if(Present && (Queue->queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			DeviceScore->QueueID = i;

			return true;
		}
	}

	HardenedLogLocation();
	return false;
}


Static bool
VulkanGetDeviceExtensions(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	if(ARRAYLEN(vkDeviceExtensions) == 0)
	{
		return true;
	}

	uint32_t ExtensionCount;
	vkEnumerateDeviceExtensionProperties(Device, NULL, &ExtensionCount, NULL);
	if(ExtensionCount == 0)
	{
		HardenedLogLocation();
		return false;
	}

	VkExtensionProperties Extensions[ExtensionCount];
	vkEnumerateDeviceExtensionProperties(Device, NULL, &ExtensionCount, Extensions);

	const char** RequiredExtension = vkDeviceExtensions;
	const char** RequiredExtensionEnd = RequiredExtension + ARRAYLEN(vkDeviceExtensions);

	do
	{
		VkExtensionProperties* Extension = Extensions;
		VkExtensionProperties* EndExtension = Extensions + ExtensionCount;

		while(1)
		{
			if(strcmp(*RequiredExtension, Extension->extensionName) == 0)
			{
				break;
			}

			if(++Extension == EndExtension)
			{
				HardenedLogLocation("%s", *RequiredExtension);
				return false;
			}
		}
	}
	while(++RequiredExtension != RequiredExtensionEnd);

	return true;
}


Static void
VulkanUpdateConstants(
	void
	)
{
	vkViewport.x = 0.0f;
	vkViewport.y = 0.0f;
	vkViewport.width = vkExtent.width;
	vkViewport.height = vkExtent.height;
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;

	vkScissor.offset.x = 0;
	vkScissor.offset.y = 0;
	vkScissor.extent = vkExtent;

	Pair Size =
	(Pair)
	{
		.W = vkExtent.width,
		.H = vkExtent.height
	};

	if(WindowSize.W != Size.W || WindowSize.H != Size.H)
	{
		WindowResizeData EventData =
		(WindowResizeData)
		{
			.OldSize = WindowSize
		};

		WindowSize = Size;
		vkConstants.WindowSize = WindowSize;

		EventData.NewSize = WindowSize;

		EventNotify(&WindowResizeTarget, &EventData);
	}
}


Static void
VulkanGetExtent(
	void
	)
{
	int Width = 0;
	int Height = 0;

	while(1)
	{
		VkResult Result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			vkPhysicalDevice, vkSurface, &vkSurfaceCapabilities);
		HardenedAssertEQ(Result, VK_SUCCESS);

		Width = vkSurfaceCapabilities.currentExtent.width;
		Height = vkSurfaceCapabilities.currentExtent.height;

		if(Width != 0 && Height != 0)
		{
			break;
		}

		MutexLock(&Mtx);
			while(!Resized)
			{
				CondVarWait(&Cond, &Mtx);
			}

			Resized = false;
		MutexUnlock(&Mtx);
	}

	Width = CLAMP(
		Width,
		vkSurfaceCapabilities.maxImageExtent.width,
		vkSurfaceCapabilities.minImageExtent.width
		);

	Height = CLAMP(
		Height,
		vkSurfaceCapabilities.maxImageExtent.height,
		vkSurfaceCapabilities.minImageExtent.height
		);

	vkExtent =
	(VkExtent2D)
	{
		.width = Width,
		.height = Height
	};

	VulkanUpdateConstants();
}


Static bool
VulkanGetDeviceSwapchain(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	uint32_t FormatCount;
	VkResult Result = vkGetPhysicalDeviceSurfaceFormatsKHR(
		Device, vkSurface, &FormatCount, NULL);

	if(Result != VK_SUCCESS)
	{
		HardenedLogLocation();
		return false;
	}

	if(FormatCount == 0)
	{
		HardenedLogLocation();
		return false;
	}

	VkSurfaceFormatKHR Formats[FormatCount];
	Result = vkGetPhysicalDeviceSurfaceFormatsKHR(
		Device, vkSurface, &FormatCount, Formats);

	if(Result != VK_SUCCESS)
	{
		HardenedLogLocation();
		return false;
	}

	VkSurfaceFormatKHR* Format = Formats;
	VkSurfaceFormatKHR* FormatEnd = Format + ARRAYLEN(Formats);

	while(1)
	{
		if(Format->format == VK_FORMAT_B8G8R8A8_SRGB &&
			Format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			break;
		}

		if(++Format == FormatEnd)
		{
			HardenedLogLocation();
			return false;
		}
	}

	return true;
}


Static bool
VulkanGetDeviceProperties(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	VkPhysicalDeviceProperties Properties;
	vkGetPhysicalDeviceProperties(Device, &Properties);

	if(Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		DeviceScore->Score += 1000;
	}

	VkSampleCountFlags Counts =
		Properties.limits.framebufferColorSampleCounts & Properties.limits.framebufferDepthSampleCounts;

	if(Counts & VK_SAMPLE_COUNT_16_BIT)
	{
		DeviceScore->Samples = VK_SAMPLE_COUNT_16_BIT;
	}
	else if(Counts & VK_SAMPLE_COUNT_8_BIT)
	{
		DeviceScore->Samples = VK_SAMPLE_COUNT_8_BIT;
	}
	else if(Counts & VK_SAMPLE_COUNT_4_BIT)
	{
		DeviceScore->Samples = VK_SAMPLE_COUNT_4_BIT;
	}
	else if(Counts & VK_SAMPLE_COUNT_2_BIT)
	{
		DeviceScore->Samples = VK_SAMPLE_COUNT_2_BIT;
	}
	else
	{
		HardenedLogLocation();
		return false;
	}

	DeviceScore->Score += DeviceScore->Samples * 16;

	if(Properties.limits.maxImageDimension2D < 1024)
	{
		HardenedLogLocation("%u\n", Properties.limits.maxImageDimension2D);
		return false;
	}

	if(Properties.limits.maxPushConstantsSize < sizeof(VkVertexConstantInput))
	{
		HardenedLogLocation("%u\n", Properties.limits.maxPushConstantsSize);
		return false;
	}

	if(Properties.limits.maxBoundDescriptorSets < 1)
	{
		HardenedLogLocation("%u\n", Properties.limits.maxBoundDescriptorSets);
		return false;
	}

	if(Properties.limits.maxPerStageDescriptorSamplers < 8)
	{
		HardenedLogLocation("%u\n", Properties.limits.maxPerStageDescriptorSamplers);
		return false;
	}

	if(Properties.limits.maxImageArrayLayers < 2048)
	{
		HardenedLogLocation("%u\n", Properties.limits.maxImageArrayLayers);
		return false;
	}

	if(Properties.limits.maxPerStageDescriptorSampledImages < 8192)
	{
		HardenedLogLocation("%u\n", Properties.limits.maxPerStageDescriptorSampledImages);
		return false;
	}

	DeviceScore->Score += Properties.limits.maxImageDimension2D;
	DeviceScore->Limits = Properties.limits;

	return true;
}


Static VkDeviceScore
VulkanGetDeviceScore(
	VkPhysicalDevice Device
	)
{
	VkDeviceScore DeviceScore = {0};

	if(!VulkanGetDeviceFeatures(Device, &DeviceScore))
	{
		goto goto_err;
	}

	if(!VulkanGetDeviceQueues(Device, &DeviceScore))
	{
		goto goto_err;
	}

	if(!VulkanGetDeviceExtensions(Device, &DeviceScore))
	{
		goto goto_err;
	}

	if(!VulkanGetDeviceSwapchain(Device, &DeviceScore))
	{
		goto goto_err;
	}

	if(!VulkanGetDeviceProperties(Device, &DeviceScore))
	{
		goto goto_err;
	}

	return DeviceScore;


	goto_err:

	DeviceScore.Score = 0;
	return DeviceScore;
}


Static void
VulkanInitDevice(
	void
	)
{
	uint32_t DeviceCount;
	vkEnumeratePhysicalDevices(vkInstance, &DeviceCount, NULL);
	HardenedAssertNEQ(DeviceCount, 0);

	VkPhysicalDevice Devices[DeviceCount];
	vkEnumeratePhysicalDevices(vkInstance, &DeviceCount, Devices);

	VkPhysicalDevice* Device = Devices;
	VkPhysicalDevice* DeviceEnd = Device + ARRAYLEN(Devices);

	VkPhysicalDevice BestDevice = NULL;
	VkDeviceScore BestDeviceScore = {0};

	do
	{
		VkDeviceScore ThisDeviceScore = VulkanGetDeviceScore(*Device);

		if(ThisDeviceScore.Score > BestDeviceScore.Score)
		{
			BestDevice = *Device;
			BestDeviceScore = ThisDeviceScore;
			vkPhysicalDevice = *Device;
		}
	}
	while(++Device != DeviceEnd);

	HardenedAssertNotNull(BestDevice);

	vkQueueID = BestDeviceScore.QueueID;
	vkSamples = BestDeviceScore.Samples;
	vkLimits = BestDeviceScore.Limits;


	float Priority = 1.0f;

	VkDeviceQueueCreateInfo Queue = {0};
	Queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	Queue.pNext = NULL;
	Queue.flags = 0;
	Queue.queueFamilyIndex = vkQueueID;
	Queue.queueCount = 1;
	Queue.pQueuePriorities = &Priority;

	VkPhysicalDeviceFeatures DeviceFeatures = {0};
	DeviceFeatures.sampleRateShading = VK_TRUE;
	DeviceFeatures.textureCompressionBC = VK_TRUE;

	VkDeviceCreateInfo DeviceInfo = {0};
	DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceInfo.pNext = NULL;
	DeviceInfo.flags = 0;
	DeviceInfo.queueCreateInfoCount = 1;
	DeviceInfo.pQueueCreateInfos = &Queue;
	DeviceInfo.enabledLayerCount = ARRAYLEN(vkLayers);
	DeviceInfo.ppEnabledLayerNames = vkLayers;
	DeviceInfo.enabledExtensionCount = ARRAYLEN(vkDeviceExtensions);
	DeviceInfo.ppEnabledExtensionNames = vkDeviceExtensions;
	DeviceInfo.pEnabledFeatures = &DeviceFeatures;

	VkResult Result = vkCreateDevice(BestDevice, &DeviceInfo, NULL, &vkDevice);
	HardenedAssertEQ(Result, VK_SUCCESS);

	volkLoadDevice(vkDevice);

	vkGetDeviceQueue(vkDevice, vkQueueID, 0, &vkQueue);

	vkGetPhysicalDeviceMemoryProperties(BestDevice, &vkMemoryProperties);


	VkCommandPoolCreateInfo PoolInfo = {0};
	PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	PoolInfo.pNext = NULL;
	PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	PoolInfo.queueFamilyIndex = vkQueueID;

	Result = vkCreateCommandPool(vkDevice, &PoolInfo, NULL, &vkCommandPool);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkCommandBufferAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.commandPool = vkCommandPool;
	AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	AllocInfo.commandBufferCount = 1;

	Result = vkAllocateCommandBuffers(vkDevice, &AllocInfo, &vkCommandBuffer);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkFenceCreateInfo FenceInfo = {0};
	FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInfo.pNext = NULL;
	FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	Result = vkCreateFence(vkDevice, &FenceInfo, NULL, &vkFence);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VulkanGetExtent();
	vkMinImageCount = MAX(
		MIN(2, vkSurfaceCapabilities.maxImageCount),
		vkSurfaceCapabilities.minImageCount
	);
	vkTransform = vkSurfaceCapabilities.currentTransform;


	uint32_t PresentModeCount;
	Result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		vkPhysicalDevice, vkSurface, &PresentModeCount, NULL);
	HardenedAssertEQ(Result, VK_SUCCESS);
	HardenedAssertNEQ(PresentModeCount, 0);

	VkPresentModeKHR PresentModes[PresentModeCount];
	Result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		vkPhysicalDevice, vkSurface, &PresentModeCount, PresentModes);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkPresentModeKHR* PresentMode = PresentModes;
	VkPresentModeKHR* PresentModeEnd = PresentModes + PresentModeCount;

	while(1)
	{
		if(*PresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		{
			vkPresentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			break;
		}

		if(++PresentMode == PresentModeEnd)
		{
			vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		}
	}
}


Static void
VulkanDestroyDevice(
	void
	)
{
	vkDestroyFence(vkDevice, vkFence, NULL);
	vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer);
	vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);

	vkDestroyDevice(vkDevice, NULL);
}


Static uint32_t
VulkanGetMemory(
	uint32_t Bits,
	VkMemoryPropertyFlags Properties
	)
{
	for(uint32_t i = 0; i < vkMemoryProperties.memoryTypeCount; ++i)
	{
		if(
			(Bits & (1 << i)) &&
			(vkMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties
			)
		{
			return i;
		}
	}

	HardenedAssertUnreachable();
}


Static void
VulkanGetBuffer(
	VkDeviceSize Size,
	VkBufferUsageFlags Usage,
	VkMemoryPropertyFlags Properties,
	VkBuffer* Buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VkBufferCreateInfo BufferInfo = {0};
	BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferInfo.pNext = NULL;
	BufferInfo.flags = 0;
	BufferInfo.size = Size;
	BufferInfo.usage = Usage;
	BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	BufferInfo.queueFamilyIndexCount = 0;
	BufferInfo.pQueueFamilyIndices = NULL;

	VkResult Result = vkCreateBuffer(vkDevice, &BufferInfo, NULL, Buffer);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkMemoryRequirements Requirements;
	vkGetBufferMemoryRequirements(vkDevice, *Buffer, &Requirements);

	VkMemoryAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.allocationSize = Requirements.size;
	AllocInfo.memoryTypeIndex = VulkanGetMemory(Requirements.memoryTypeBits, Properties);

	Result = vkAllocateMemory(vkDevice, &AllocInfo, NULL, BufferMemory);
	HardenedAssertEQ(Result, VK_SUCCESS);

	vkBindBufferMemory(vkDevice, *Buffer, *BufferMemory, 0);
}


Static void
VulkanGetStagingBuffer(
	VkDeviceSize Size,
	VkBuffer* Buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VulkanGetBuffer(Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer, BufferMemory);
}


Static void
VulkanGetVertexBuffer(
	VkDeviceSize Size,
	VkBuffer* Buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VulkanGetBuffer(Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Buffer, BufferMemory);
}


Static void
VulkanGetDrawIndirectBuffer(
	VkDeviceSize Size,
	VkBuffer* Buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VulkanGetBuffer(Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Buffer, BufferMemory);
}


Static void
VulkanDestroyCopyBuffer(
	void
	)
{
	vkFreeMemory(vkDevice, vkCopyBufferMemory, NULL);
	vkDestroyBuffer(vkDevice, vkCopyBuffer, NULL);
}


Static void
VulkanCopyToBuffer(
	VkBuffer Buffer,
	const void* Data,
	VkDeviceSize Size
	)
{
	if(!Size)
	{
		return;
	}

	VulkanBeginCommandBuffer();

	VulkanDestroyCopyBuffer();

	VulkanGetStagingBuffer(Size, &vkCopyBuffer, &vkCopyBufferMemory);

	void* Memory;

	VkResult Result = vkMapMemory(vkDevice, vkCopyBufferMemory, 0, VK_WHOLE_SIZE, 0, &Memory);
	HardenedAssertEQ(Result, VK_SUCCESS);

	memcpy(Memory, Data, Size);
	vkUnmapMemory(vkDevice, vkCopyBufferMemory);

	VkBufferCopy Copy = {0};
	Copy.srcOffset = 0;
	Copy.dstOffset = 0;
	Copy.size = Size;

	vkCmdCopyBuffer(vkCommandBuffer, vkCopyBuffer, Buffer, 1, &Copy);

	VulkanEndCommandBuffer();
}


Static void
VulkanCopyTextureToImage(
	Image* Image,
	DDSTexture* Texture
	)
{
	VulkanBeginCommandBuffer();

	VulkanDestroyCopyBuffer();

	VkDeviceSize Size = DDSDataSize(Texture);
	uint8_t* Data = &Texture->Data[0];

	VulkanGetStagingBuffer(Size, &vkCopyBuffer, &vkCopyBufferMemory);

	void* Memory;

	VkResult Result = vkMapMemory(vkDevice, vkCopyBufferMemory, 0, VK_WHOLE_SIZE, 0, &Memory);
	HardenedAssertEQ(Result, VK_SUCCESS);

	memcpy(Memory, Data, Size);
	vkUnmapMemory(vkDevice, vkCopyBufferMemory);

	VkBufferImageCopy Copies[Image->Layers];

	uint32_t Layer = 0;
	VkBufferImageCopy* Copy = Copies;
	VkBufferImageCopy* CopyEnd = Copies + ARRAYLEN(Copies);

	while(1)
	{
		*Copy = (VkBufferImageCopy){0};
		Copy->bufferOffset = DDSOffset(Texture, Layer);
		Copy->bufferRowLength = 0;
		Copy->bufferImageHeight = 0;
		Copy->imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Copy->imageSubresource.mipLevel = 0;
		Copy->imageSubresource.baseArrayLayer = Layer;
		Copy->imageSubresource.layerCount = 1;
		Copy->imageOffset.x = 0;
		Copy->imageOffset.y = 0;
		Copy->imageOffset.z = 0;
		Copy->imageExtent.width = Image->Width;
		Copy->imageExtent.height = Image->Height;
		Copy->imageExtent.depth = 1;

		if(++Copy == CopyEnd)
		{
			break;
		}

		++Layer;
	}

	vkCmdCopyBufferToImage(vkCommandBuffer, vkCopyBuffer, Image->Image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ARRAYLEN(Copies), Copies);

	VulkanEndCommandBuffer();
}


Static void
VulkanTransitionImageLayout(
	Image* Image,
	VkImageLayout From,
	VkImageLayout To
	)
{
	VulkanBeginCommandBuffer();

	VkPipelineStageFlags SourceStage;
	VkPipelineStageFlags DestinationStage;

	VkImageMemoryBarrier Barrier = {0};
	Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	Barrier.pNext = NULL;
	Barrier.srcAccessMask = 0;
	Barrier.dstAccessMask = 0;

	if(From == VK_IMAGE_LAYOUT_UNDEFINED && To == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		Barrier.srcAccessMask = 0;
		Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(From == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && To == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		AssertUnreachable();
	}

	Barrier.oldLayout = From;
	Barrier.newLayout = To;
	Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	Barrier.image = Image->Image;
	Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	Barrier.subresourceRange.baseMipLevel = 0;
	Barrier.subresourceRange.levelCount = 1;
	Barrier.subresourceRange.baseArrayLayer = 0;
	Barrier.subresourceRange.layerCount = Image->Layers;

	vkCmdPipelineBarrier(vkCommandBuffer, SourceStage, DestinationStage, 0, 0, NULL, 0, NULL, 1, &Barrier);

	VulkanEndCommandBuffer();
}


Static void
VulkanCreateImage(
	Image* Image
	)
{
	DDSTexture* Texture = NULL;

	switch(Image->Type)
	{

	case IMAGE_TYPE_DEPTH_STENCIL:
	{
		Image->Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		Image->Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		Image->Samples = vkSamples;

		Image->Width = vkExtent.width;
		Image->Height = vkExtent.height;
		Image->Layers = 1;

		break;
	}

	case IMAGE_TYPE_MULTISAMPLED:
	{
		Image->Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		Image->Usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		Image->Samples = vkSamples;

		Image->Width = vkExtent.width;
		Image->Height = vkExtent.height;
		Image->Layers = 1;

		break;
	}

	case IMAGE_TYPE_TEXTURE:
	{
		Texture = DDSLoad(Image->Path);

		Image->Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		Image->Usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		Image->Samples = VK_SAMPLE_COUNT_1_BIT;

		Image->Width = Texture->Width;
		Image->Height = Texture->Height;
		Image->Layers = Texture->ArraySize;

		break;
	}

	}

	VkImageCreateInfo ImageInfo = {0};
	ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageInfo.pNext = NULL;
	ImageInfo.flags = 0;
	ImageInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageInfo.format = Image->Format;
	ImageInfo.extent.width = Image->Width;
	ImageInfo.extent.height = Image->Height;
	ImageInfo.extent.depth = 1;
	ImageInfo.mipLevels = 1;
	ImageInfo.arrayLayers = Image->Layers;
	ImageInfo.samples = Image->Samples;
	ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageInfo.usage = Image->Usage;
	ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageInfo.queueFamilyIndexCount = 0;
	ImageInfo.pQueueFamilyIndices = NULL;
	ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult Result = vkCreateImage(vkDevice, &ImageInfo, NULL, &Image->Image);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkMemoryRequirements Requirements;
	vkGetImageMemoryRequirements(vkDevice, Image->Image, &Requirements);

	VkMemoryAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.allocationSize = Requirements.size;
	AllocInfo.memoryTypeIndex = VulkanGetMemory(Requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	Result = vkAllocateMemory(vkDevice, &AllocInfo, NULL, &Image->Memory);
	HardenedAssertEQ(Result, VK_SUCCESS);

	Result = vkBindImageMemory(vkDevice, Image->Image, Image->Memory, 0);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkImageViewCreateInfo ViewInfo = {0};
	ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ViewInfo.pNext = NULL;
	ViewInfo.flags = 0;
	ViewInfo.image = Image->Image;
	ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	ViewInfo.format = Image->Format;
	ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.subresourceRange.aspectMask = Image->Aspect;
	ViewInfo.subresourceRange.baseMipLevel = 0;
	ViewInfo.subresourceRange.levelCount = 1;
	ViewInfo.subresourceRange.baseArrayLayer = 0;
	ViewInfo.subresourceRange.layerCount = Image->Layers;

	Result = vkCreateImageView(vkDevice, &ViewInfo, NULL, &Image->View);
	HardenedAssertEQ(Result, VK_SUCCESS);

	if(Image->Type == IMAGE_TYPE_TEXTURE)
	{
		VulkanTransitionImageLayout(Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VulkanCopyTextureToImage(Image, Texture);

		DDSFree(Texture);

		VulkanTransitionImageLayout(Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}


Static void
VulkanDestroyImage(
	Image* Image
	)
{
	vkFreeMemory(vkDevice, Image->Memory, NULL);
	vkDestroyImageView(vkDevice, Image->View, NULL);
	vkDestroyImage(vkDevice, Image->Image, NULL);
}


Static void
VulkanInitImages(
	void
	)
{
	VulkanCreateImage(&vkDepthBuffer);
	VulkanCreateImage(&vkMultisampling);
}


Static void
VulkanDestroyImages(
	void
	)
{
	VulkanDestroyImage(&vkMultisampling);
	VulkanDestroyImage(&vkDepthBuffer);
}


Static VkShaderModule
VulkanCreateShader(
	const char* Path
	)
{
	FileFile File;
	bool Status = FileRead(Path, &File);
	HardenedAssertTrue(Status);

	const uint32_t* Code = (const uint32_t*) File.Buffer;

	VkShaderModuleCreateInfo ShaderInfo = {0};
	ShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderInfo.pNext = NULL;
	ShaderInfo.flags = 0;
	ShaderInfo.codeSize = File.Length;
	ShaderInfo.pCode = Code;

	VkShaderModule Shader;

	VkResult Result = vkCreateShaderModule(vkDevice, &ShaderInfo, NULL, &Shader);
	HardenedAssertEQ(Result, VK_SUCCESS);

	FileFree(File);

	return Shader;
}


Static void
VulkanDestroyShader(
	VkShaderModule Shader
	)
{
	vkDestroyShaderModule(vkDevice, Shader, NULL);
}


Static void
VulkanInitPipeline(
	void
	)
{
	VkAttachmentReference ColorAttachmentRef = {0};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthAttachmentRef = {0};
	DepthAttachmentRef.attachment = 1;
	DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference MultisamplingRef = {0};
	MultisamplingRef.attachment = 2;
	MultisamplingRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription Subpass = {0};
	Subpass.flags = 0;
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.inputAttachmentCount = 0;
	Subpass.pInputAttachments = NULL;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentRef;
	Subpass.pResolveAttachments = &MultisamplingRef;
	Subpass.pDepthStencilAttachment = &DepthAttachmentRef;
	Subpass.preserveAttachmentCount = 0;
	Subpass.pPreserveAttachments = NULL;

	VkSubpassDependency Dependency = {0};
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0;
	Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	Dependency.srcAccessMask = 0;
	Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	Dependency.dependencyFlags = 0;

	VkAttachmentDescription Attachments[3] = {0};

	Attachments[0].flags = 0;
	Attachments[0].format = vkMultisampling.Format;
	Attachments[0].samples = vkSamples;
	Attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	Attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	Attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	Attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	Attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	Attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	Attachments[1].flags = 0;
	Attachments[1].format = vkDepthBuffer.Format;
	Attachments[1].samples = vkSamples;
	Attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	Attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	Attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	Attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	Attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	Attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	Attachments[2].flags = 0;
	Attachments[2].format = VK_FORMAT_B8G8R8A8_SRGB;
	Attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
	Attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	Attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	Attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	Attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	Attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	Attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkRenderPassCreateInfo RenderPassInfo = {0};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.pNext = NULL;
	RenderPassInfo.flags = 0;
	RenderPassInfo.attachmentCount = ARRAYLEN(Attachments);
	RenderPassInfo.pAttachments = Attachments;
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &Subpass;
	RenderPassInfo.dependencyCount = 1;
	RenderPassInfo.pDependencies = &Dependency;

	VkResult Result = vkCreateRenderPass(vkDevice, &RenderPassInfo, NULL, &vkRenderPass);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkShaderModule VertexModule = VulkanCreateShader("vert.spv");
	VkShaderModule FragmentModule = VulkanCreateShader("frag.spv");

	VkPipelineShaderStageCreateInfo Stages[2] = {0};

	Stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Stages[0].pNext = NULL;
	Stages[0].flags = 0;
	Stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	Stages[0].module = VertexModule;
	Stages[0].pName = "main";
	Stages[0].pSpecializationInfo = NULL;

	Stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Stages[1].pNext = NULL;
	Stages[1].flags = 0;
	Stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	Stages[1].module = FragmentModule;
	Stages[1].pName = "main";
	Stages[1].pSpecializationInfo = NULL;

	VkDynamicState DynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo DynamicState = {0};
	DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	DynamicState.pNext = NULL;
	DynamicState.flags = 0;
	DynamicState.dynamicStateCount = ARRAYLEN(DynamicStates);
	DynamicState.pDynamicStates = DynamicStates;

	VkVertexInputBindingDescription VertexBindings[2] = {0};

	VertexBindings[0].binding = 0;
	VertexBindings[0].stride = sizeof(VkVertexVertexInput);
	VertexBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VertexBindings[1].binding = 1;
	VertexBindings[1].stride = sizeof(VkVertexInstanceInput);
	VertexBindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	VkVertexInputAttributeDescription Attributes[] =
	{
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(VkVertexVertexInput, Position)
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(VkVertexVertexInput, TexCoord)
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(VkVertexInstanceInput, Position)
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(VkVertexInstanceInput, Dimensions)
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.offset = offsetof(VkVertexInstanceInput, WhiteColor)
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(VkVertexInstanceInput, WhiteDepth)
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.offset = offsetof(VkVertexInstanceInput, BlackColor)
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(VkVertexInstanceInput, BlackDepth)
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R16G16_UINT,
			.offset = offsetof(VkVertexInstanceInput, Texture)
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(VkVertexInstanceInput, Rotation)
		},
		{
			.location = 10,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(VkVertexInstanceInput, TexScale)
		},
		{
			.location = 11,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(VkVertexInstanceInput, TexOffset)
		}
	};

	VkPipelineVertexInputStateCreateInfo VertexInput = {0};
	VertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInput.pNext = NULL;
	VertexInput.flags = 0;
	VertexInput.vertexBindingDescriptionCount = ARRAYLEN(VertexBindings);
	VertexInput.pVertexBindingDescriptions = VertexBindings;
	VertexInput.vertexAttributeDescriptionCount = ARRAYLEN(Attributes);
	VertexInput.pVertexAttributeDescriptions = Attributes;

	VkPipelineInputAssemblyStateCreateInfo InputAssembly = {0};
	InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssembly.pNext = NULL;
	InputAssembly.flags = 0;
	InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	InputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo ViewportState = {0};
	ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportState.pNext = NULL;
	ViewportState.flags = 0;
	ViewportState.viewportCount = 1;
	ViewportState.pViewports = &vkViewport;
	ViewportState.scissorCount = 1;
	ViewportState.pScissors = &vkScissor;

	VkPipelineRasterizationStateCreateInfo Rasterizer = {0};
	Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	Rasterizer.pNext = NULL;
	Rasterizer.flags = 0;
	Rasterizer.depthClampEnable = VK_FALSE;
	Rasterizer.rasterizerDiscardEnable = VK_FALSE;
	Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	Rasterizer.cullMode = VK_CULL_MODE_NONE;
	Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	Rasterizer.depthBiasEnable = VK_FALSE;
	Rasterizer.depthBiasConstantFactor = 0.0f;
	Rasterizer.depthBiasClamp = 0.0f;
	Rasterizer.depthBiasSlopeFactor = 0.0f;
	Rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo Multisampling = {0};
	Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	Multisampling.pNext = NULL;
	Multisampling.flags = 0;
	Multisampling.rasterizationSamples = vkSamples;
	Multisampling.sampleShadingEnable = VK_TRUE;
	Multisampling.minSampleShading = 1.0f;
	Multisampling.pSampleMask = NULL;
	Multisampling.alphaToCoverageEnable = VK_FALSE;
	Multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo DepthStencil = {0};
	DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencil.pNext = NULL;
	DepthStencil.flags = 0;
	DepthStencil.depthTestEnable = VK_TRUE;
	DepthStencil.depthWriteEnable = VK_TRUE;
	DepthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
	DepthStencil.depthBoundsTestEnable = VK_FALSE;
	DepthStencil.stencilTestEnable = VK_FALSE;
	DepthStencil.front = (VkStencilOpState){0};
	DepthStencil.back = (VkStencilOpState){0};
	DepthStencil.minDepthBounds = 0.0f;
	DepthStencil.maxDepthBounds = 0.0f;

	VkPipelineColorBlendAttachmentState BlendingAttachment = {0};
	BlendingAttachment.blendEnable = VK_TRUE;
	BlendingAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	BlendingAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	BlendingAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	BlendingAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	BlendingAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	BlendingAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	BlendingAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo Blending = {0};
	Blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	Blending.pNext = NULL;
	Blending.flags = 0;
	Blending.logicOpEnable = VK_FALSE;
	Blending.logicOp = VK_LOGIC_OP_COPY;
	Blending.attachmentCount = 1;
	Blending.pAttachments = &BlendingAttachment;
	Blending.blendConstants[0] = 0.0f;
	Blending.blendConstants[1] = 0.0f;
	Blending.blendConstants[2] = 0.0f;
	Blending.blendConstants[3] = 0.0f;

	VkDescriptorSetLayoutBinding Bindings[1] = {0};

	Bindings[0].binding = 0;
	Bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	Bindings[0].descriptorCount = ARRAYLEN(vkTextures);
	Bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	Bindings[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo Descriptors = {0};
	Descriptors.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	Descriptors.pNext = NULL;
	Descriptors.flags = 0;
	Descriptors.bindingCount = ARRAYLEN(Bindings);
	Descriptors.pBindings = Bindings;

	Result = vkCreateDescriptorSetLayout(vkDevice, &Descriptors, NULL, &vkDescriptors);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkPushConstantRange PushConstants[1] = {0};

	PushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	PushConstants[0].offset = 0;
	PushConstants[0].size = sizeof(VkVertexConstantInput);

	VkPipelineLayoutCreateInfo LayoutInfo = {0};
	LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	LayoutInfo.pNext = NULL;
	LayoutInfo.flags = 0;
	LayoutInfo.setLayoutCount = 1;
	LayoutInfo.pSetLayouts = &vkDescriptors;
	LayoutInfo.pushConstantRangeCount = ARRAYLEN(PushConstants);
	LayoutInfo.pPushConstantRanges = PushConstants;

	Result = vkCreatePipelineLayout(vkDevice, &LayoutInfo, NULL, &vkPipelineLayout);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkGraphicsPipelineCreateInfo PipelineInfo = {0};
	PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineInfo.pNext = NULL;
	PipelineInfo.flags = 0;
	PipelineInfo.stageCount = 2;
	PipelineInfo.pStages = Stages;
	PipelineInfo.pVertexInputState = &VertexInput;
	PipelineInfo.pInputAssemblyState = &InputAssembly;
	PipelineInfo.pTessellationState = NULL;
	PipelineInfo.pViewportState = &ViewportState;
	PipelineInfo.pRasterizationState = &Rasterizer;
	PipelineInfo.pMultisampleState = &Multisampling;
	PipelineInfo.pDepthStencilState = &DepthStencil;
	PipelineInfo.pColorBlendState = &Blending;
	PipelineInfo.pDynamicState = &DynamicState;
	PipelineInfo.layout = vkPipelineLayout;
	PipelineInfo.renderPass = vkRenderPass;
	PipelineInfo.subpass = 0;
	PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	PipelineInfo.basePipelineIndex = -1;

	Result = vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &PipelineInfo, NULL, &vkPipeline);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VulkanDestroyShader(VertexModule);
	VulkanDestroyShader(FragmentModule);
}


Static void
VulkanDestroyPipeline(
	void
	)
{
	vkDestroyPipeline(vkDevice, vkPipeline, NULL);
	vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
	vkDestroyDescriptorSetLayout(vkDevice, vkDescriptors, NULL);
	vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
}


Static void
VulkanInitConsts(
	void
	)
{
	VkBarrier* Barrier = vkBarriers;
	VkBarrier* BarrierEnd = Barrier + ARRAYLEN(vkBarriers);

	do
	{
		VkSemaphoreCreateInfo SemaphoreInfo = {0};
		SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		SemaphoreInfo.pNext = NULL;
		SemaphoreInfo.flags = 0;

		VkSemaphore* Semaphore = Barrier->Semaphores;
		VkSemaphore* SemaphoreEnd = Semaphore + ARRAYLEN(Barrier->Semaphores);

		do
		{
			VkResult Result = vkCreateSemaphore(vkDevice, &SemaphoreInfo, NULL, Semaphore);
			HardenedAssertEQ(Result, VK_SUCCESS);
		}
		while(++Semaphore != SemaphoreEnd);


		VkFenceCreateInfo FenceInfo = {0};
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.pNext = NULL;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence* Fence = Barrier->Fences;
		VkFence* FenceEnd = Barrier->Fences + ARRAYLEN(Barrier->Fences);

		do
		{
			VkResult Result = vkCreateFence(vkDevice, &FenceInfo, NULL, Fence);
			HardenedAssertEQ(Result, VK_SUCCESS);
		}
		while(++Fence != FenceEnd);
	}
	while(++Barrier != BarrierEnd);


	VkCommandBuffer CommandBuffers[ARRAYLEN(vkFrames)];

	VkCommandBufferAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.commandPool = vkCommandPool;
	AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	AllocInfo.commandBufferCount = ARRAYLEN(CommandBuffers);

	VkResult Result = vkAllocateCommandBuffers(vkDevice, &AllocInfo, CommandBuffers);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkFrame* Frame = vkFrames;
	VkFrame* FrameEnd = Frame + ARRAYLEN(vkFrames);

	VkCommandBuffer* CommandBuffer = CommandBuffers;

	while(1)
	{
		Frame->CommandBuffer = *CommandBuffer;

		if(++Frame == FrameEnd)
		{
			break;
		}

		++CommandBuffer;
	}
}


Static void
VulkanDestroyConsts(
	void
	)
{
	VkFrame* Frame = vkFrames;
	VkFrame* FrameEnd = Frame + ARRAYLEN(vkFrames);

	VkCommandBuffer CommandBuffers[ARRAYLEN(vkFrames)];
	VkCommandBuffer* CommandBuffer = CommandBuffers;

	while(1)
	{
		*CommandBuffer = Frame->CommandBuffer;

		if(++Frame == FrameEnd)
		{
			break;
		}

		++CommandBuffer;
	}

	vkFreeCommandBuffers(vkDevice, vkCommandPool, ARRAYLEN(CommandBuffers), CommandBuffers);


	VkBarrier* Barrier = vkBarriers;
	VkBarrier* BarrierEnd = Barrier + ARRAYLEN(vkBarriers);

	do
	{
		VkFence* Fence = Barrier->Fences;
		VkFence* FenceEnd = Barrier->Fences + ARRAYLEN(Barrier->Fences);

		do
		{
			vkDestroyFence(vkDevice, *Fence, NULL);
		}
		while(++Fence != FenceEnd);


		VkSemaphore* Semaphore = Barrier->Semaphores;
		VkSemaphore* SemaphoreEnd = Barrier->Semaphores + ARRAYLEN(Barrier->Semaphores);

		do
		{
			vkDestroySemaphore(vkDevice, *Semaphore, NULL);
		}
		while(++Semaphore != SemaphoreEnd);
	}
	while(++Barrier != BarrierEnd);
}


Static void
VulkanInitFrames(
	void
	)
{
	uint32_t ImageCount;
	VkResult Result = vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &ImageCount, NULL);
	HardenedAssertEQ(Result, VK_SUCCESS);

	if(vkImageCount == 0)
	{
		AssertNEQ(ImageCount, 0);
		AssertLE(ImageCount, ARRAYLEN(vkFrames));
		vkImageCount = ImageCount;
	}
	else
	{
		AssertEQ(ImageCount, vkImageCount);
	}

	VkImage Images[vkImageCount];
	Result = vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkImageCount, Images);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkFrame* Frame = vkFrames;
	VkFrame* FrameEnd = Frame + vkImageCount;

	VkImage* Image = Images;

	while(1)
	{
		Frame->Image = *Image;


		VkImageViewCreateInfo ViewInfo = {0};
		ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewInfo.pNext = NULL;
		ViewInfo.flags = 0;
		ViewInfo.image = *Image;
		ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ViewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
		ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ViewInfo.subresourceRange.baseMipLevel = 0;
		ViewInfo.subresourceRange.levelCount = 1;
		ViewInfo.subresourceRange.baseArrayLayer = 0;
		ViewInfo.subresourceRange.layerCount = 1;

		VkResult Result = vkCreateImageView(vkDevice, &ViewInfo, NULL, &Frame->ImageView);
		HardenedAssertEQ(Result, VK_SUCCESS);


		VkImageView Attachments[] =
		{
			vkMultisampling.View,
			vkDepthBuffer.View,
			Frame->ImageView
		};

		VkFramebufferCreateInfo FramebufferInfo = {0};
		FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferInfo.pNext = NULL;
		FramebufferInfo.flags = 0;
		FramebufferInfo.renderPass = vkRenderPass;
		FramebufferInfo.attachmentCount = ARRAYLEN(Attachments);
		FramebufferInfo.pAttachments = Attachments;
		FramebufferInfo.width = vkExtent.width;
		FramebufferInfo.height = vkExtent.height;
		FramebufferInfo.layers = 1;

		Result = vkCreateFramebuffer(vkDevice, &FramebufferInfo, NULL, &Frame->Framebuffer);
		HardenedAssertEQ(Result, VK_SUCCESS);


		if(++Frame == FrameEnd)
		{
			break;
		}

		++Image;
	}
}


Static void
VulkanDestroyFrames(
	void
	)
{
	VkFrame* Frame = vkFrames;
	VkFrame* FrameEnd = Frame + vkImageCount;

	do
	{
		vkDestroyFramebuffer(vkDevice, Frame->Framebuffer, NULL);
		vkDestroyImageView(vkDevice, Frame->ImageView, NULL);
	}
	while(++Frame != FrameEnd);
}


Static VkSwapchainKHR
VulkanCreateSwapchain(
	VkSwapchainKHR OldSwapchain
	)
{
	VkSwapchainCreateInfoKHR SwapchainInfo = {0};
	SwapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainInfo.pNext = NULL;
	SwapchainInfo.flags = 0;
	SwapchainInfo.surface = vkSurface;
	SwapchainInfo.minImageCount = vkMinImageCount;
	SwapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	SwapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	SwapchainInfo.imageExtent = vkExtent;
	SwapchainInfo.imageArrayLayers = 1;
	SwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapchainInfo.queueFamilyIndexCount = 0;
	SwapchainInfo.pQueueFamilyIndices = NULL;
	SwapchainInfo.preTransform = vkTransform;
	SwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainInfo.presentMode = vkPresentMode;
	SwapchainInfo.clipped = VK_TRUE;
	SwapchainInfo.oldSwapchain = OldSwapchain;

	VkSwapchainKHR Swapchain;

	VkResult Result = vkCreateSwapchainKHR(vkDevice, &SwapchainInfo, NULL, &Swapchain);
	HardenedAssertEQ(Result, VK_SUCCESS);

	return Swapchain;
}


Static void
VulkanInitSwapchain(
	void
	)
{
	vkSwapchain = VulkanCreateSwapchain(VK_NULL_HANDLE);

	VulkanInitFrames();
}


Static void
VulkanDestroySwapchain(
	void
	)
{
	VulkanDestroyFrames();

	vkDestroySwapchainKHR(vkDevice, vkSwapchain, NULL);
}


Static void
VulkanReinitSwapchain(
	void
	)
{
	VkSwapchainKHR NewSwapchain = VulkanCreateSwapchain(vkSwapchain);

	VulkanDestroySwapchain();

	vkSwapchain = NewSwapchain;

	VulkanInitFrames();
}


Static void
VulkanInitTextures(
	void
	)
{
	VkSamplerCreateInfo SamplerInfo = {0};
	SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInfo.pNext = NULL;
	SamplerInfo.flags = 0;
	SamplerInfo.magFilter = VK_FILTER_NEAREST;
	SamplerInfo.minFilter = VK_FILTER_NEAREST;
	SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	SamplerInfo.mipLodBias = 0.0f;
	SamplerInfo.anisotropyEnable = VK_FALSE;
	SamplerInfo.maxAnisotropy = 0.0f;
	SamplerInfo.compareEnable = VK_FALSE;
	SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerInfo.minLod = 0.0f;
	SamplerInfo.maxLod = 0.0f;
	SamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	SamplerInfo.unnormalizedCoordinates = VK_FALSE;

	VkResult Result = vkCreateSampler(vkDevice, &SamplerInfo, NULL, &vkSampler);
	HardenedAssertEQ(Result, VK_SUCCESS);


	Image* Texture = vkTextures;
	Image* TextureEnd = vkTextures + ARRAYLEN(vkTextures);

	const TexFile* TextureFile = TextureFiles;

	while(Texture != TextureEnd)
	{
		Texture->Path = TextureFile->Path;
		Texture->Format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		Texture->Type = IMAGE_TYPE_TEXTURE;

		VulkanCreateImage(Texture);

		++TextureFile;
		++Texture;
	}


	VkDescriptorPoolSize PoolSizes[1] = {0};

	PoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	PoolSizes[0].descriptorCount = ARRAYLEN(vkTextures);

	VkDescriptorPoolCreateInfo DescriptorInfo = {0};
	DescriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorInfo.pNext = NULL;
	DescriptorInfo.flags = 0;
	DescriptorInfo.maxSets = 1;
	DescriptorInfo.poolSizeCount = ARRAYLEN(PoolSizes);
	DescriptorInfo.pPoolSizes = PoolSizes;

	Result = vkCreateDescriptorPool(vkDevice, &DescriptorInfo, NULL, &vkDescriptorPool);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkDescriptorSetAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.descriptorPool = vkDescriptorPool;
	AllocInfo.descriptorSetCount = 1;
	AllocInfo.pSetLayouts = &vkDescriptors;

	Result = vkAllocateDescriptorSets(vkDevice, &AllocInfo, &vkDescriptorSet);
	HardenedAssertEQ(Result, VK_SUCCESS);


	VkDescriptorImageInfo ImageInfos[ARRAYLEN(vkTextures)] = {0};
	VkWriteDescriptorSet DescriptorWrites[ARRAYLEN(vkTextures)] = {0};

	Texture = vkTextures;

	for(uint32_t i = 0; i < ARRAYLEN(vkTextures); ++i) {
		ImageInfos[i].sampler = vkSampler;
		ImageInfos[i].imageView = Texture->View;
		ImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		DescriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[i].pNext = NULL;
		DescriptorWrites[i].dstSet = vkDescriptorSet;
		DescriptorWrites[i].dstBinding = 0;
		DescriptorWrites[i].dstArrayElement = i;
		DescriptorWrites[i].descriptorCount = 1;
		DescriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorWrites[i].pImageInfo = ImageInfos + i;
		DescriptorWrites[i].pBufferInfo = NULL;
		DescriptorWrites[i].pTexelBufferView = NULL;

		++Texture;
	}

	vkUpdateDescriptorSets(vkDevice, ARRAYLEN(vkTextures), DescriptorWrites, 0, NULL);
}


Static void
VulkanDestroyTextures(
	void
	)
{
	vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);


	Image* Texture = vkTextures;
	Image* TextureEnd = vkTextures + ARRAYLEN(vkTextures);

	while(Texture != TextureEnd)
	{
		VulkanDestroyImage(Texture);

		++Texture;
	}

	vkDestroySampler(vkDevice, vkSampler, NULL);
}


Static void
VulkanInitVertex(
	void
	)
{
	VulkanGetVertexBuffer(sizeof(vkVertexVertexInput), &vkVertexVertexInputBuffer, &vkVertexVertexInputMemory);
	VulkanCopyToBuffer(vkVertexVertexInputBuffer, vkVertexVertexInput, sizeof(vkVertexVertexInput));

	VulkanGetVertexBuffer(sizeof(VkVertexInstanceInput) * GAME_CONST_MAX_TEXTURES,
		&vkVertexInstanceInputBuffer, &vkVertexInstanceInputMemory);

	DrawDataBuffer = AllocMalloc(sizeof(DrawData) * GAME_CONST_MAX_TEXTURES);
	HardenedAssertNotNull(DrawDataBuffer);

	VulkanGetDrawIndirectBuffer(sizeof(VkDrawIndirectCommand), &vkDrawCountBuffer, &vkDrawCountMemory);
}


Static void
VulkanDestroyVertex(
	void
	)
{
	vkFreeMemory(vkDevice, vkDrawCountMemory, NULL);
	vkDestroyBuffer(vkDevice, vkDrawCountBuffer, NULL);

	AllocFree(sizeof(DrawData) * GAME_CONST_MAX_TEXTURES, DrawDataBuffer);

	vkFreeMemory(vkDevice, vkVertexInstanceInputMemory, NULL);
	vkDestroyBuffer(vkDevice, vkVertexInstanceInputBuffer, NULL);

	vkFreeMemory(vkDevice, vkVertexVertexInputMemory, NULL);
	vkDestroyBuffer(vkDevice, vkVertexVertexInputBuffer, NULL);
}


Static void
VulkanInitSync(
	void
	)
{
	MutexInit(&Mtx);
	CondVarInit(&Cond);
	MutexInit(&WindowMtx);
}


Static void
VulkanDestroySync(
	void
	)
{
	MutexDestroy(&WindowMtx);
	CondVarDestroy(&Cond);
	MutexDestroy(&Mtx);
}


Static void
VulkanRecordCommands(
	void
	)
{
	VkDeviceSize Offset = 0;

	VkClearValue ClearValues[2] = {0};
	ClearValues[0].color = (VkClearColorValue){{ 0.0f, 0.0f, 0.0f, 0.0f }};
	ClearValues[1].depthStencil = (VkClearDepthStencilValue){ 0.0f, 0 };


	VkFrame* Frame = vkFrames;
	VkFrame* FrameEnd = Frame + vkImageCount;

	do
	{
		VkResult Result = vkResetCommandBuffer(Frame->CommandBuffer, 0);
		HardenedAssertEQ(Result, VK_SUCCESS);

		VkCommandBufferBeginInfo BeginInfo = {0};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.pNext = NULL;
		BeginInfo.flags = 0;
		BeginInfo.pInheritanceInfo = NULL;

		Result = vkBeginCommandBuffer(Frame->CommandBuffer, &BeginInfo);
		HardenedAssertEQ(Result, VK_SUCCESS);

		VkRenderPassBeginInfo RenderPassInfo = {0};
		RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPassInfo.pNext = NULL;
		RenderPassInfo.renderPass = vkRenderPass;
		RenderPassInfo.framebuffer = Frame->Framebuffer;
		RenderPassInfo.renderArea.offset.x = 0;
		RenderPassInfo.renderArea.offset.y = 0;
		RenderPassInfo.renderArea.extent = vkExtent;
		RenderPassInfo.clearValueCount = ARRAYLEN(ClearValues);
		RenderPassInfo.pClearValues = ClearValues;

		vkCmdBeginRenderPass(Frame->CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(Frame->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		vkCmdSetViewport(Frame->CommandBuffer, 0, 1, &vkViewport);
		vkCmdSetScissor(Frame->CommandBuffer, 0, 1, &vkScissor);

		vkCmdBindVertexBuffers(Frame->CommandBuffer, 0, 1, &vkVertexVertexInputBuffer, &Offset);
		vkCmdBindVertexBuffers(Frame->CommandBuffer, 1, 1, &vkVertexInstanceInputBuffer, &Offset);

		vkCmdBindDescriptorSets(Frame->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, NULL);

		vkCmdPushConstants(Frame->CommandBuffer, vkPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vkConstants), &vkConstants);

		vkCmdDrawIndirect(Frame->CommandBuffer, vkDrawCountBuffer, 0, 1, 0);

		vkCmdEndRenderPass(Frame->CommandBuffer);

		Result = vkEndCommandBuffer(Frame->CommandBuffer);
		HardenedAssertEQ(Result, VK_SUCCESS);
	}
	while(++Frame != FrameEnd);
}


Static void
VulkanRecreateSwapchain(
	void
	)
{
	vkDeviceWaitIdle(vkDevice);

	VulkanDestroyImages();
	VulkanGetExtent();
	VulkanInitImages();

	VulkanReinitSwapchain();

	VulkanRecordCommands();
}


void
WindowAddDrawData(
	const DrawData* Data
	)
{
	DrawDataBuffer[DrawDataCount++] = *Data;
}


Static void
VulkanDraw(
	void
	)
{
	VkResult Result = vkWaitForFences(vkDevice, 1, vkBarrier->Fences + FENCE_IN_FLIGHT, VK_TRUE, UINT64_MAX);
	HardenedAssertEQ(Result, VK_SUCCESS);

	uint32_t ImageIndex;
	Result = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX,
		vkBarrier->Semaphores[SEMAPHORE_IMAGE_AVAILABLE], VK_NULL_HANDLE, &ImageIndex);
	if(Result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		VulkanRecreateSwapchain();

		return;
	}

	VkFrame* Frame = vkFrames + ImageIndex;

	Result = vkResetFences(vkDevice, 1, vkBarrier->Fences + FENCE_IN_FLIGHT);
	HardenedAssertEQ(Result, VK_SUCCESS);


	DrawDataCount = 0;
	MutexLock(&WindowMtx);
		EventNotify(&WindowDrawTarget, &((WindowDrawData){0}));
	MutexUnlock(&WindowMtx);

	uint32_t TotalSize = sizeof(DrawData) * DrawDataCount;
	VulkanCopyToBuffer(vkVertexInstanceInputBuffer, DrawDataBuffer, TotalSize);


	VkDrawIndirectCommand Command = {0};
	Command.vertexCount = ARRAYLEN(vkVertexVertexInput);
	Command.instanceCount = DrawDataCount;
	Command.firstVertex = 0;
	Command.firstInstance = 0;

	VulkanCopyToBuffer(vkDrawCountBuffer, &Command, sizeof(Command));


	VkPipelineStageFlags WaitStages[] =
	{
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	VkSubmitInfo SubmitInfo = {0};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pNext = NULL;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = vkBarrier->Semaphores + SEMAPHORE_IMAGE_AVAILABLE;
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &Frame->CommandBuffer;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = vkBarrier->Semaphores + SEMAPHORE_RENDER_FINISHED;

	Result = vkQueueSubmit(vkQueue, 1, &SubmitInfo, vkBarrier->Fences[FENCE_IN_FLIGHT]);
	HardenedAssertEQ(Result, VK_SUCCESS);

	VkPresentInfoKHR PresentInfo = {0};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = NULL;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = vkBarrier->Semaphores + SEMAPHORE_RENDER_FINISHED;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &vkSwapchain;
	PresentInfo.pImageIndices = &ImageIndex;
	PresentInfo.pResults = NULL;

	Result = vkQueuePresentKHR(vkQueue, &PresentInfo);

	if(Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
	{
		VulkanRecreateSwapchain();
	}

	if(++vkBarrier == vkBarriers + ARRAYLEN(vkBarriers))
	{
		vkBarrier = vkBarriers;
	}
}


void
WindowInit(
	void
	)
{
	VulkanInitSDL();
	VulkanInitInstance();
	VulkanInitSurface();
	VulkanInitDevice();
	VulkanInitImages();
	VulkanInitPipeline();
	VulkanInitConsts();
	VulkanInitSwapchain();
	VulkanInitTextures();
	VulkanInitVertex();
	VulkanInitSync();

	VulkanRecordCommands();
}


Static void
VulkanThreadFN(
	void* Data
	)
{
	(void) Data;

	while(atomic_load_explicit(&vkShouldRun, memory_order_acquire))
	{
		VulkanDraw();

		if(FirstFrame)
		{
			FirstFrame = false;
			SDL_ShowWindow(Window);
			SDL_RaiseWindow(Window);
		}
	}
}


Static void
WindowInterrupt(
	int Signal
	)
{
	WindowQuit();
}


void
WindowRun(
	void
	)
{
	signal(SIGINT, WindowInterrupt);

	atomic_store_explicit(&vkShouldRun, 1, memory_order_relaxed);

	ThreadInit(&vkThread, VulkanThreadFN, NULL);

	while(1)
	{
		SDL_Event Event;
		SDL_WaitEvent(&Event);

		if(Event.type == SDL_EVENT_QUIT)
		{
			break;
		}

		WindowOnEvent(&Event);
	}

	atomic_store_explicit(&vkShouldRun, 0, memory_order_release);
}


void
WindowFree(
	void
	)
{
	ThreadWait(vkThread);

	vkDeviceWaitIdle(vkDevice);

	VulkanDestroyCopyBuffer();

	VulkanDestroySync();
	VulkanDestroyVertex();
	VulkanDestroyTextures();
	VulkanDestroySwapchain();
	VulkanDestroyConsts();
	VulkanDestroyPipeline();
	VulkanDestroyImages();
	VulkanDestroyDevice();
	VulkanDestroySurface();
	VulkanDestroyInstance();
	VulkanDestroySDL();
}
