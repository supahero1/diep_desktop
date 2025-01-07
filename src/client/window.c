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

#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/client/window.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/client/tex/base.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/client/window/dds.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <DiepDesktop/client/window/volk.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <signal.h>
#include <string.h>
#include <stdatomic.h>

#define VULKAN_MAX_IMAGES 8


private SDL_PropertiesID WindowProps;
private SDL_Window* Window;
private SDL_Cursor* Cursors[WINDOW_CURSOR__COUNT];
private window_cursor_t CurrentCursor = WINDOW_CURSOR_DEFAULT;
private pair_t WindowSize;
private ipair_t OldWindowPosition;
private ipair_t OldWindowSize;
private pair_t MousePosition;
private bool Fullscreen = false;
private bool FirstFrame = true;



private const char* vkInstanceExtensions[] =
{
#ifndef NDEBUG
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

private const char* vkLayers[] =
{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation"
#endif
};

private VkInstance vkInstance;


private VkSurfaceKHR vkSurface;
private VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;


private const char* vkDeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


private uint32_t vkQueueID;
private VkSampleCountFlagBits vkSamples;
private VkPhysicalDeviceLimits vkLimits;

private VkPhysicalDevice vkPhysicalDevice;
private VkDevice vkDevice;
private VkQueue vkQueue;
private VkPhysicalDeviceMemoryProperties vkMemoryProperties;

private VkCommandPool vkCommandPool;
private VkCommandBuffer vkCommandBuffer;
private VkFence vkFence;

private VkExtent2D vkExtent;
private uint32_t vkMinImageCount;
private VkSurfaceTransformFlagBitsKHR vkTransform;
private VkPresentModeKHR vkPresentMode;


private VkSwapchainKHR vkSwapchain;
private uint32_t vkImageCount;

typedef struct VkFrame
{
	VkImage Image;
	VkImageView ImageView;
	VkFramebuffer Framebuffer;
	VkCommandBuffer CommandBuffer;
}
VkFrame;

private VkFrame vkFrames[VULKAN_MAX_IMAGES];


typedef enum VkBarrierSemaphore
{
	BARRIER_SEMAPHORE_IMAGE_AVAILABLE,
	BARRIER_SEMAPHORE_RENDER_FINISHED,
	BARRIER_SEMAPHORE__COUNT
}
VkBarrierSemaphore;

typedef enum VkBarrierFence
{
	BARRIER_FENCE_IN_FLIGHT,
	BARRIER_FENCE__COUNT
}
VkBarrierFence;

typedef struct VkBarrier
{
	VkSemaphore Semaphores[BARRIER_SEMAPHORE__COUNT];
	VkFence Fences[BARRIER_FENCE__COUNT];
}
VkBarrier;

private VkBarrier vkBarriers[VULKAN_MAX_IMAGES];
private VkBarrier* vkBarrier = vkBarriers;


private VkViewport vkViewport;
private VkRect2D vkScissor;


private VkBuffer vkCopyBuffer;
private VkDeviceMemory vkCopyBufferMemory;


private VkSampler vkSampler;


typedef enum ImageType
{
	IMAGE_TYPE_DEPTH_STENCIL,
	IMAGE_TYPE_MULTISAMPLED,
	IMAGE_TYPE_TEXTURE
}
ImageType;

typedef struct Image
{
	const char* path;

	uint32_t width;
	uint32_t height;
	uint32_t Layers;

	VkFormat format;
	ImageType type;

	VkImage Image;
	VkImageView View;
	VkDeviceMemory Memory;

	VkImageAspectFlags Aspect;
	VkImageUsageFlags Usage;
	VkSampleCountFlagBits Samples;
}
Image;

private Image vkDepthBuffer =
{
	.format = VK_FORMAT_D32_SFLOAT,
	.type = IMAGE_TYPE_DEPTH_STENCIL
};

private Image vkMultisampling =
{
	.format = VK_FORMAT_B8G8R8A8_SRGB,
	.type = IMAGE_TYPE_MULTISAMPLED
};

private Image vkTextures[TEXTURES_NUM];


typedef struct VkVertexVertexInput
{
	vec2 pos;
	vec2 TexCoord;
}
VkVertexVertexInput;

private const VkVertexVertexInput vkVertexVertexInput[] =
{
	{ { -0.5f, -0.5f }, { 0.0f, 0.0f } },
	{ {  0.5f, -0.5f }, { 1.0f, 0.0f } },
	{ { -0.5f,  0.5f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f }, { 1.0f, 1.0f } },
};

private VkBuffer vkVertexVertexInputBuffer;
private VkDeviceMemory vkVertexVertexInputMemory;

private VkBuffer vkVertexInstanceInputBuffer;
private VkDeviceMemory vkVertexInstanceInputMemory;

private VkBuffer vkDrawCountBuffer;
private VkDeviceMemory vkDrawCountMemory;


typedef struct VkVertexConstantInput
{
	pair_t WindowSize;
}
VkVertexConstantInput;

private VkVertexConstantInput vkConstants;


private VkDescriptorSetLayout vkDescriptors;
private VkRenderPass vkRenderPass;
private VkPipelineLayout vkPipelineLayout;
private VkPipeline vkPipeline;
private VkDescriptorPool vkDescriptorPool;
private VkDescriptorSet vkDescriptorSet;


private thread_t vkThread;
private _Atomic uint8_t vkShouldRun;
private sync_mtx_t mtx;
private sync_cond_t Cond;
private bool Resized;
private sync_mtx_t WindowMtx;


event_target_t window_resize_target;
event_target_t window_focus_target;
event_target_t window_blur_target;
event_target_t window_key_down_target;
event_target_t window_key_up_target;
event_target_t window_text_target;
event_target_t window_mouse_down_target;
event_target_t window_mouse_up_target;
event_target_t window_mouse_move_target;
event_target_t window_mouse_scroll_target;


private window_draw_event_data_t* DrawDataBuffer;
private uint32_t DrawDataCount;

event_target_t window_draw_target;



pair_t
window_get_size(
	void
	)
{
	return WindowSize;
}


pair_t
window_get_mouse_pos(
	void
	)
{
	return MousePosition;
}


#ifndef NDEBUG

private VkDebugUtilsMessengerEXT vkDebugMessenger;

private VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* data,
	void* UserData
	)
{
	puts(data->pMessage);

	// if(Severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	// {
	// 	abort();
	// }

	return VK_FALSE;
}

#endif /* NDEBUG */


private void
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


private void
VulkanToggleFullscreen(
	void
	)
{
	if(Fullscreen)
	{
		bool status = SDL_SetWindowFullscreen(Window, false);
		assert_true(status);

		status = SDL_SetWindowSize(Window, OldWindowSize.w, OldWindowSize.h);
		assert_true(status);

		status = SDL_SetWindowPosition(Window, OldWindowPosition.x, OldWindowPosition.y);
		assert_true(status);
	}
	else
	{
		bool status = SDL_GetWindowSize(Window, &OldWindowSize.w, &OldWindowSize.h);
		assert_true(status);

		status = SDL_GetWindowPosition(Window, &OldWindowPosition.x, &OldWindowPosition.y);
		assert_true(status);

		status = SDL_SetWindowFullscreen(Window, true);
		assert_true(status);
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
window_set_cursor(
	window_cursor_t cursor
	)
{
	if(cursor != CurrentCursor)
	{
		CurrentCursor = cursor;
		SDL_SetCursor(Cursors[cursor]);
	}
}


String
window_get_clipboard(
	void
	)
{
	char* str = SDL_GetClipboardText();
	if(!str)
	{
		return (String){0};
	}

	uint32_t len = strlen(str) + 1;

	char* NewText = alloc_malloc(len);
	if(!NewText)
	{
		SDL_free(str);
		return (String){0};
	}

	memcpy(NewText, str, len);

	SDL_free(str);

	return (String){ .str = NewText, .len = len };
}


void
window_set_clipboard(
	const char* str
	)
{
	SDL_SetClipboardText(str);
}


private void
WindowQuit(
	void
	)
{
	SDL_PushEvent(&(SDL_Event){ .type = SDL_EVENT_QUIT });
}



#define ___(x) WINDOW_KEY_##x
#define __(x) SDLK_##x
#define _(x) case __(x): return ___(x);

private window_key_t
MapSDLKey(
	int key
	)
{
	switch(key)
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


private window_key_mod_t
MapSDLMod(
	int Mod
	)
{
	window_key_mod_t Result = 0;

	if(Mod & SDL_KMOD_SHIFT) Result |= WINDOW_KEY_MOD_SHIFT;
	if(Mod & SDL_KMOD_CTRL) Result |= WINDOW_KEY_MOD_CTRL;
	if(Mod & SDL_KMOD_ALT) Result |= WINDOW_KEY_MOD_ALT;
	if(Mod & SDL_KMOD_GUI) Result |= WINDOW_KEY_MOD_GUI;
	if(Mod & SDL_KMOD_CAPS) Result |= WINDOW_KEY_MOD_CAPS_LOCK;

	return Result;
}


private window_button_t
MapSDLbutton(
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


	sync_mtx_lock(&WindowMtx);


	switch(Event->type)
	{

	case SDL_EVENT_WINDOW_RESIZED:
	{
		if(!Resized)
		{
			Resized = true;
			sync_cond_wake(&Cond);
		}

		break;
	}

	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	{
		event_target_fire(&window_focus_target, &((window_focus_event_data_t){0}));
		break;
	}

	case SDL_EVENT_WINDOW_FOCUS_LOST:
	{
		event_target_fire(&window_blur_target, &((window_blur_event_data_t){0}));
		break;
	}

	case SDL_EVENT_KEY_DOWN:
	{
		SDL_KeyboardEvent key = Event->key;

		if((key.mod & SDL_KMOD_CTRL) && (key.key == SDLK_W || key.key == SDLK_R))
		{
			WindowQuit();
		}

		if(key.key == SDLK_F11 && !key.repeat)
		{
			VulkanToggleFullscreen();
		}

		window_key_down_event_data_t event_data =
		(window_key_down_event_data_t)
		{
			.key = MapSDLKey(key.key),
			.mods = MapSDLMod(key.mod),
			.repeat = key.repeat
		};

		event_target_fire(&window_key_down_target, &event_data);

		break;
	}

	case SDL_EVENT_KEY_UP:
	{
		SDL_KeyboardEvent key = Event->key;

		window_key_up_event_data_t event_data =
		(window_key_up_event_data_t)
		{
			.key = MapSDLKey(key.key),
			.mods = MapSDLMod(key.mod)
		};

		event_target_fire(&window_key_up_target, &event_data);

		break;
	}

	case SDL_EVENT_TEXT_INPUT:
	{
		SDL_TextInputEvent str = Event->text;

		if(!str.text)
		{
			break;
		}

		window_text_event_data_t event_data =
		(window_text_event_data_t)
		{
			.str = str.text,
			.len = strlen(str.text)
		};

		event_target_fire(&window_text_target, &event_data);

		break;
	}

	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	{
		SDL_MouseButtonEvent button = Event->button;

		float HalfWidth  = WindowSize.w * 0.5f;
		float HalfHeight = WindowSize.h * 0.5f;

		pair_t EventMousePosition =
		(pair_t)
		{
			.x = MACRO_CLAMP_SYM(button.x - HalfWidth , HalfWidth ),
			.y = MACRO_CLAMP_SYM(button.y - HalfHeight, HalfHeight)
		};

		assert_lt(fabsf(EventMousePosition.x - MousePosition.x), 0.01f);
		assert_lt(fabsf(EventMousePosition.y - MousePosition.y), 0.01f);

		window_mouse_down_event_data_t event_data =
		(window_mouse_down_event_data_t)
		{
			.button = MapSDLbutton(button.button),
			.pos = MousePosition,
			.clicks = button.clicks
		};

		event_target_fire(&window_mouse_down_target, &event_data);

		break;
	}

	case SDL_EVENT_MOUSE_BUTTON_UP:
	{
		SDL_MouseButtonEvent button = Event->button;

		float HalfWidth  = WindowSize.w * 0.5f;
		float HalfHeight = WindowSize.h * 0.5f;

		pair_t EventMousePosition =
		(pair_t)
		{
			.x = MACRO_CLAMP_SYM(button.x - HalfWidth , HalfWidth ),
			.y = MACRO_CLAMP_SYM(button.y - HalfHeight, HalfHeight)
		};

		assert_lt(fabsf(EventMousePosition.x - MousePosition.x), 0.01f);
		assert_lt(fabsf(EventMousePosition.y - MousePosition.y), 0.01f);

		window_mouse_up_event_data_t event_data =
		(window_mouse_up_event_data_t)
		{
			.button = MapSDLbutton(button.button),
			.pos = MousePosition
		};

		event_target_fire(&window_mouse_up_target, &event_data);

		break;
	}

	case SDL_EVENT_MOUSE_MOTION:
	{
		SDL_MouseMotionEvent Motion = Event->motion;

		window_mouse_move_event_data_t event_data =
		(window_mouse_move_event_data_t)
		{
			.old_pos = MousePosition
		};

		float HalfWidth  = WindowSize.w * 0.5f;
		float HalfHeight = WindowSize.h * 0.5f;

		MousePosition =
		(pair_t)
		{
			.x = MACRO_CLAMP_SYM(Motion.x - HalfWidth , HalfWidth ),
			.y = MACRO_CLAMP_SYM(Motion.y - HalfHeight, HalfHeight)
		};

		event_data.new_pos = MousePosition;

		event_target_fire(&window_mouse_move_target, &event_data);

		break;
	}

	case SDL_EVENT_MOUSE_WHEEL:
	{
		SDL_MouseWheelEvent Wheel = Event->wheel;

		window_mouse_scroll_event_data_t event_data =
		(window_mouse_scroll_event_data_t)
		{
			.offset_y = Event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -Wheel.y : Wheel.y
		};

		event_target_fire(&window_mouse_scroll_target, &event_data);

		break;
	}

	default: break;

	}


	sync_mtx_unlock(&WindowMtx);
}


void
window_start_typing(
	void
	)
{
	SDL_StartTextInput(Window);
}


void
window_stop_typing(
	void
	)
{
	SDL_StopTextInput(Window);
}


private void
VulkanInitSDL(
	void
	)
{
	bool status = SDL_InitSubSystem(SDL_INIT_VIDEO);
	hard_assert_true(status);

	WindowProps = SDL_CreateProperties();
	assert_neq(WindowProps, 0);

	status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
	assert_true(status);

	status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
	assert_true(status);

	status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	assert_true(status);

	status = SDL_SetBooleanProperty(WindowProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
	assert_true(status);

	status = SDL_SetNumberProperty(WindowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, GAME_CONST_DEFAULT_WINDOW_WIDTH);
	assert_true(status);

	status = SDL_SetNumberProperty(WindowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, GAME_CONST_DEFAULT_WINDOW_HEIGHT);
	assert_true(status);

	status = SDL_SetStringProperty(WindowProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Game");
	assert_true(status);

	status = SDL_SetBooleanProperty(WindowProps, SDL_HINT_FORCE_RAISEWINDOW, true);
	assert_true(status);

	Window = SDL_CreateWindowWithProperties(WindowProps);
	if(!Window)
	{
		SDLError();
		hard_assert_unreachable();
	}

	status = SDL_SetWindowMinimumSize(Window, 480, 270);
	assert_true(status);

	// SDL_Rect MouseBound = { 0, 0, 1280, 720 };
	// SDL_SetWindowMouseRect(Window, &MouseBound);

	Cursors[WINDOW_CURSOR_DEFAULT] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	Cursors[WINDOW_CURSOR_TYPING] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
	Cursors[WINDOW_CURSOR_POINTING] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
}


private void
VulkanDestroySDL(
	void
	)
{
	SDL_Cursor** Cursor = Cursors;
	SDL_Cursor** CursorEnd = Cursor + MACRO_ARRAY_LEN(Cursors);

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


private void
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
	InstanceInfo.enabledLayerCount = MACRO_ARRAY_LEN(vkLayers);
	InstanceInfo.ppEnabledLayerNames = vkLayers;

	uint32_t SDLExtensionCount;
	const char* const* SDLExtensions = SDL_Vulkan_GetInstanceExtensions(&SDLExtensionCount);

	uint32_t ExtensionCount = SDLExtensionCount + MACRO_ARRAY_LEN(vkInstanceExtensions);
	const char* Extensions[ExtensionCount];

	const char** Extension = Extensions;
	for(uint32_t i = 0; i < SDLExtensionCount; ++i)
	{
		*(Extension++) = SDLExtensions[i];
	}
	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(vkInstanceExtensions); ++i)
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
	hard_assert_eq(Result, VK_SUCCESS);

	volkLoadInstance(vkInstance);

#ifndef NDEBUG
	Result = vkCreateDebugUtilsMessengerEXT(vkInstance, &DebugInfo, NULL, &vkDebugMessenger);
	assert_eq(Result, VK_SUCCESS);
#endif
}


private void
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


private void
VulkanInitSurface(
	void
	)
{
	bool status = SDL_Vulkan_CreateSurface(Window, vkInstance, NULL, &vkSurface);
	if(status == false)
	{
		SDLError();
		hard_assert_unreachable();
	}
}


private void
VulkanDestroySurface(
	void
	)
{
	vkDestroySurfaceKHR(vkInstance, vkSurface, NULL);
}


private void
VulkanBeginCommandBuffer(
	void
	)
{
	VkResult Result = vkWaitForFences(vkDevice, 1, &vkFence, VK_TRUE, UINT64_MAX);
	hard_assert_eq(Result, VK_SUCCESS);

	Result = vkResetFences(vkDevice, 1, &vkFence);
	hard_assert_eq(Result, VK_SUCCESS);

	Result = vkResetCommandBuffer(vkCommandBuffer, 0);
	hard_assert_eq(Result, VK_SUCCESS);

	VkCommandBufferBeginInfo BeginInfo = {0};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.pNext = NULL;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	BeginInfo.pInheritanceInfo = NULL;

	Result = vkBeginCommandBuffer(vkCommandBuffer, &BeginInfo);
	hard_assert_eq(Result, VK_SUCCESS);
}


private void
VulkanEndCommandBuffer(
	void
	)
{
	VkResult Result = vkEndCommandBuffer(vkCommandBuffer);
	hard_assert_eq(Result, VK_SUCCESS);

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


private bool
VulkanGetDeviceFeatures(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	VkPhysicalDeviceFeatures Features;
	vkGetPhysicalDeviceFeatures(Device, &Features);

	if(!Features.sampleRateShading)
	{
		hard_assert_log();
		return false;
	}

	if(!Features.textureCompressionBC)
	{
		hard_assert_log();
		return false;
	}

	return true;
}


private bool
VulkanGetDeviceQueues(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	uint32_t QueueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueCount, NULL);

	if(QueueCount == 0)
	{
		hard_assert_log();
		return false;
	}

	VkQueueFamilyProperties Queues[QueueCount];
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueCount, Queues);

	VkQueueFamilyProperties* queue = Queues;

	for(uint32_t i = 0; i < QueueCount; ++i, ++queue)
	{
		VkBool32 Present;
		VkResult Result = vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, vkSurface, &Present);

		if(Result != VK_SUCCESS)
		{
			hard_assert_log();
			return false;
		}

		if(Present && (queue->queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			DeviceScore->QueueID = i;

			return true;
		}
	}

	hard_assert_log();
	return false;
}


private bool
VulkanGetDeviceExtensions(
	VkPhysicalDevice Device,
	VkDeviceScore* DeviceScore
	)
{
	if(MACRO_ARRAY_LEN(vkDeviceExtensions) == 0)
	{
		return true;
	}

	uint32_t ExtensionCount;
	vkEnumerateDeviceExtensionProperties(Device, NULL, &ExtensionCount, NULL);
	if(ExtensionCount == 0)
	{
		hard_assert_log();
		return false;
	}

	VkExtensionProperties Extensions[ExtensionCount];
	vkEnumerateDeviceExtensionProperties(Device, NULL, &ExtensionCount, Extensions);

	const char** RequiredExtension = vkDeviceExtensions;
	const char** RequiredExtensionEnd = RequiredExtension + MACRO_ARRAY_LEN(vkDeviceExtensions);

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
				hard_assert_log("%s", *RequiredExtension);
				return false;
			}
		}
	}
	while(++RequiredExtension != RequiredExtensionEnd);

	return true;
}


private void
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

	pair_t size =
	(pair_t)
	{
		.w = vkExtent.width,
		.h = vkExtent.height
	};

	if(WindowSize.w != size.w || WindowSize.h != size.h)
	{
		window_resize_event_data_t event_data =
		(window_resize_event_data_t)
		{
			.old_size = WindowSize
		};

		WindowSize = size;
		vkConstants.WindowSize = WindowSize;

		event_data.new_size = WindowSize;

		event_target_fire(&window_resize_target, &event_data);
	}
}


private void
VulkanGetExtent(
	void
	)
{
	int width = 0;
	int height = 0;

	while(1)
	{
		VkResult Result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			vkPhysicalDevice, vkSurface, &vkSurfaceCapabilities);
		hard_assert_eq(Result, VK_SUCCESS);

		width = vkSurfaceCapabilities.currentExtent.width;
		height = vkSurfaceCapabilities.currentExtent.height;

		if(width != 0 && height != 0)
		{
			break;
		}

		sync_mtx_lock(&mtx);
			while(!Resized)
			{
				sync_cond_wait(&Cond, &mtx);
			}

			Resized = false;
		sync_mtx_unlock(&mtx);
	}

	width = MACRO_CLAMP(
		width,
		vkSurfaceCapabilities.maxImageExtent.width,
		vkSurfaceCapabilities.minImageExtent.width
		);

	height = MACRO_CLAMP(
		height,
		vkSurfaceCapabilities.maxImageExtent.height,
		vkSurfaceCapabilities.minImageExtent.height
		);

	vkExtent =
	(VkExtent2D)
	{
		.width = width,
		.height = height
	};

	VulkanUpdateConstants();
}


private bool
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
		hard_assert_log();
		return false;
	}

	if(FormatCount == 0)
	{
		hard_assert_log();
		return false;
	}

	VkSurfaceFormatKHR Formats[FormatCount];
	Result = vkGetPhysicalDeviceSurfaceFormatsKHR(
		Device, vkSurface, &FormatCount, Formats);

	if(Result != VK_SUCCESS)
	{
		hard_assert_log();
		return false;
	}

	VkSurfaceFormatKHR* format = Formats;
	VkSurfaceFormatKHR* FormatEnd = format + MACRO_ARRAY_LEN(Formats);

	while(1)
	{
		if(format->format == VK_FORMAT_B8G8R8A8_SRGB &&
			format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			break;
		}

		if(++format == FormatEnd)
		{
			hard_assert_log();
			return false;
		}
	}

	return true;
}


private bool
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
		hard_assert_log();
		return false;
	}

	DeviceScore->Score += DeviceScore->Samples * 16;

	if(Properties.limits.maxImageDimension2D < 1024)
	{
		hard_assert_log("%u\n", Properties.limits.maxImageDimension2D);
		return false;
	}

	if(Properties.limits.maxPushConstantsSize < sizeof(VkVertexConstantInput))
	{
		hard_assert_log("%u\n", Properties.limits.maxPushConstantsSize);
		return false;
	}

	if(Properties.limits.maxBoundDescriptorSets < 1)
	{
		hard_assert_log("%u\n", Properties.limits.maxBoundDescriptorSets);
		return false;
	}

	if(Properties.limits.maxPerStageDescriptorSamplers < 8)
	{
		hard_assert_log("%u\n", Properties.limits.maxPerStageDescriptorSamplers);
		return false;
	}

	if(Properties.limits.maxImageArrayLayers < 2048)
	{
		hard_assert_log("%u\n", Properties.limits.maxImageArrayLayers);
		return false;
	}

	if(Properties.limits.maxPerStageDescriptorSampledImages < 8192)
	{
		hard_assert_log("%u\n", Properties.limits.maxPerStageDescriptorSampledImages);
		return false;
	}

	DeviceScore->Score += Properties.limits.maxImageDimension2D;
	DeviceScore->Limits = Properties.limits;

	return true;
}


private VkDeviceScore
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


private void
VulkanInitDevice(
	void
	)
{
	uint32_t DeviceCount;
	vkEnumeratePhysicalDevices(vkInstance, &DeviceCount, NULL);
	hard_assert_neq(DeviceCount, 0);

	VkPhysicalDevice Devices[DeviceCount];
	vkEnumeratePhysicalDevices(vkInstance, &DeviceCount, Devices);

	VkPhysicalDevice* Device = Devices;
	VkPhysicalDevice* DeviceEnd = Device + MACRO_ARRAY_LEN(Devices);

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

	hard_assert_not_null(BestDevice);

	vkQueueID = BestDeviceScore.QueueID;
	vkSamples = BestDeviceScore.Samples;
	vkLimits = BestDeviceScore.Limits;


	float Priority = 1.0f;

	VkDeviceQueueCreateInfo queue = {0};
	queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue.pNext = NULL;
	queue.flags = 0;
	queue.queueFamilyIndex = vkQueueID;
	queue.queueCount = 1;
	queue.pQueuePriorities = &Priority;

	VkPhysicalDeviceFeatures DeviceFeatures = {0};
	DeviceFeatures.sampleRateShading = VK_TRUE;
	DeviceFeatures.textureCompressionBC = VK_TRUE;

	VkDeviceCreateInfo DeviceInfo = {0};
	DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceInfo.pNext = NULL;
	DeviceInfo.flags = 0;
	DeviceInfo.queueCreateInfoCount = 1;
	DeviceInfo.pQueueCreateInfos = &queue;
	DeviceInfo.enabledLayerCount = MACRO_ARRAY_LEN(vkLayers);
	DeviceInfo.ppEnabledLayerNames = vkLayers;
	DeviceInfo.enabledExtensionCount = MACRO_ARRAY_LEN(vkDeviceExtensions);
	DeviceInfo.ppEnabledExtensionNames = vkDeviceExtensions;
	DeviceInfo.pEnabledFeatures = &DeviceFeatures;

	VkResult Result = vkCreateDevice(BestDevice, &DeviceInfo, NULL, &vkDevice);
	hard_assert_eq(Result, VK_SUCCESS);

	volkLoadDevice(vkDevice);

	vkGetDeviceQueue(vkDevice, vkQueueID, 0, &vkQueue);

	vkGetPhysicalDeviceMemoryProperties(BestDevice, &vkMemoryProperties);


	VkCommandPoolCreateInfo PoolInfo = {0};
	PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	PoolInfo.pNext = NULL;
	PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	PoolInfo.queueFamilyIndex = vkQueueID;

	Result = vkCreateCommandPool(vkDevice, &PoolInfo, NULL, &vkCommandPool);
	hard_assert_eq(Result, VK_SUCCESS);


	VkCommandBufferAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.commandPool = vkCommandPool;
	AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	AllocInfo.commandBufferCount = 1;

	Result = vkAllocateCommandBuffers(vkDevice, &AllocInfo, &vkCommandBuffer);
	hard_assert_eq(Result, VK_SUCCESS);


	VkFenceCreateInfo FenceInfo = {0};
	FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInfo.pNext = NULL;
	FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	Result = vkCreateFence(vkDevice, &FenceInfo, NULL, &vkFence);
	hard_assert_eq(Result, VK_SUCCESS);


	VulkanGetExtent();
	vkMinImageCount = MACRO_MAX(
		MACRO_MIN(2, vkSurfaceCapabilities.maxImageCount),
		vkSurfaceCapabilities.minImageCount
	);
	vkTransform = vkSurfaceCapabilities.currentTransform;


	uint32_t PresentModeCount;
	Result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		vkPhysicalDevice, vkSurface, &PresentModeCount, NULL);
	hard_assert_eq(Result, VK_SUCCESS);
	hard_assert_neq(PresentModeCount, 0);

	VkPresentModeKHR PresentModes[PresentModeCount];
	Result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		vkPhysicalDevice, vkSurface, &PresentModeCount, PresentModes);
	hard_assert_eq(Result, VK_SUCCESS);

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


private void
VulkanDestroyDevice(
	void
	)
{
	vkDestroyFence(vkDevice, vkFence, NULL);
	vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer);
	vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);

	vkDestroyDevice(vkDevice, NULL);
}


private uint32_t
VulkanGetMemory(
	uint32_t bits,
	VkMemoryPropertyFlags Properties
	)
{
	for(uint32_t i = 0; i < vkMemoryProperties.memoryTypeCount; ++i)
	{
		if(
			(bits & (1 << i)) &&
			(vkMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties
			)
		{
			return i;
		}
	}

	hard_assert_unreachable();
}


private void
VulkanGetBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags Usage,
	VkMemoryPropertyFlags Properties,
	VkBuffer* buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VkBufferCreateInfo BufferInfo = {0};
	BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferInfo.pNext = NULL;
	BufferInfo.flags = 0;
	BufferInfo.size = size;
	BufferInfo.usage = Usage;
	BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	BufferInfo.queueFamilyIndexCount = 0;
	BufferInfo.pQueueFamilyIndices = NULL;

	VkResult Result = vkCreateBuffer(vkDevice, &BufferInfo, NULL, buffer);
	hard_assert_eq(Result, VK_SUCCESS);

	VkMemoryRequirements Requirements;
	vkGetBufferMemoryRequirements(vkDevice, *buffer, &Requirements);

	VkMemoryAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.allocationSize = Requirements.size;
	AllocInfo.memoryTypeIndex = VulkanGetMemory(Requirements.memoryTypeBits, Properties);

	Result = vkAllocateMemory(vkDevice, &AllocInfo, NULL, BufferMemory);
	hard_assert_eq(Result, VK_SUCCESS);

	vkBindBufferMemory(vkDevice, *buffer, *BufferMemory, 0);
}


private void
VulkanGetStagingBuffer(
	VkDeviceSize size,
	VkBuffer* buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VulkanGetBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, BufferMemory);
}


private void
VulkanGetVertexBuffer(
	VkDeviceSize size,
	VkBuffer* buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VulkanGetBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, BufferMemory);
}


private void
VulkanGetDrawIndirectBuffer(
	VkDeviceSize size,
	VkBuffer* buffer,
	VkDeviceMemory* BufferMemory
	)
{
	VulkanGetBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, BufferMemory);
}


private void
VulkanDestroyCopyBuffer(
	void
	)
{
	vkFreeMemory(vkDevice, vkCopyBufferMemory, NULL);
	vkDestroyBuffer(vkDevice, vkCopyBuffer, NULL);
}


private void
VulkanCopyToBuffer(
	VkBuffer buffer,
	const void* data,
	VkDeviceSize size
	)
{
	if(!size)
	{
		return;
	}

	VulkanBeginCommandBuffer();

	VulkanDestroyCopyBuffer();

	VulkanGetStagingBuffer(size, &vkCopyBuffer, &vkCopyBufferMemory);

	void* Memory;

	VkResult Result = vkMapMemory(vkDevice, vkCopyBufferMemory, 0, VK_WHOLE_SIZE, 0, &Memory);
	hard_assert_eq(Result, VK_SUCCESS);

	memcpy(Memory, data, size);
	vkUnmapMemory(vkDevice, vkCopyBufferMemory);

	VkBufferCopy Copy = {0};
	Copy.srcOffset = 0;
	Copy.dstOffset = 0;
	Copy.size = size;

	vkCmdCopyBuffer(vkCommandBuffer, vkCopyBuffer, buffer, 1, &Copy);

	VulkanEndCommandBuffer();
}


private void
VulkanCopyTextureToImage(
	Image* Image,
	dds_tex_t* tex
	)
{
	VulkanBeginCommandBuffer();

	VulkanDestroyCopyBuffer();

	VkDeviceSize size = dds_data_size(tex);
	uint8_t* data = &tex->data[0];

	VulkanGetStagingBuffer(size, &vkCopyBuffer, &vkCopyBufferMemory);

	void* Memory;

	VkResult Result = vkMapMemory(vkDevice, vkCopyBufferMemory, 0, VK_WHOLE_SIZE, 0, &Memory);
	hard_assert_eq(Result, VK_SUCCESS);

	memcpy(Memory, data, size);
	vkUnmapMemory(vkDevice, vkCopyBufferMemory);

	VkBufferImageCopy Copies[Image->Layers];

	uint32_t layer = 0;
	VkBufferImageCopy* Copy = Copies;
	VkBufferImageCopy* CopyEnd = Copies + MACRO_ARRAY_LEN(Copies);

	while(1)
	{
		*Copy = (VkBufferImageCopy){0};
		Copy->bufferOffset = dds_offset(tex, layer);
		Copy->bufferRowLength = 0;
		Copy->bufferImageHeight = 0;
		Copy->imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Copy->imageSubresource.mipLevel = 0;
		Copy->imageSubresource.baseArrayLayer = layer;
		Copy->imageSubresource.layerCount = 1;
		Copy->imageOffset.x = 0;
		Copy->imageOffset.y = 0;
		Copy->imageOffset.z = 0;
		Copy->imageExtent.width = Image->width;
		Copy->imageExtent.height = Image->height;
		Copy->imageExtent.depth = 1;

		if(++Copy == CopyEnd)
		{
			break;
		}

		++layer;
	}

	vkCmdCopyBufferToImage(vkCommandBuffer, vkCopyBuffer, Image->Image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, MACRO_ARRAY_LEN(Copies), Copies);

	VulkanEndCommandBuffer();
}


private void
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
		assert_unreachable();
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


private void
VulkanCreateImage(
	Image* Image
	)
{
	dds_tex_t* tex = NULL;

	switch(Image->type)
	{

	case IMAGE_TYPE_DEPTH_STENCIL:
	{
		Image->Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		Image->Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		Image->Samples = vkSamples;

		Image->width = vkExtent.width;
		Image->height = vkExtent.height;
		Image->Layers = 1;

		break;
	}

	case IMAGE_TYPE_MULTISAMPLED:
	{
		Image->Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		Image->Usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		Image->Samples = vkSamples;

		Image->width = vkExtent.width;
		Image->height = vkExtent.height;
		Image->Layers = 1;

		break;
	}

	case IMAGE_TYPE_TEXTURE:
	{
		tex = dds_load(Image->path);

		Image->Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		Image->Usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		Image->Samples = VK_SAMPLE_COUNT_1_BIT;

		Image->width = tex->width;
		Image->height = tex->height;
		Image->Layers = tex->array_size;

		break;
	}

	}

	VkImageCreateInfo ImageInfo = {0};
	ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageInfo.pNext = NULL;
	ImageInfo.flags = 0;
	ImageInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageInfo.format = Image->format;
	ImageInfo.extent.width = Image->width;
	ImageInfo.extent.height = Image->height;
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
	hard_assert_eq(Result, VK_SUCCESS);

	VkMemoryRequirements Requirements;
	vkGetImageMemoryRequirements(vkDevice, Image->Image, &Requirements);

	VkMemoryAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.allocationSize = Requirements.size;
	AllocInfo.memoryTypeIndex = VulkanGetMemory(Requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	Result = vkAllocateMemory(vkDevice, &AllocInfo, NULL, &Image->Memory);
	hard_assert_eq(Result, VK_SUCCESS);

	Result = vkBindImageMemory(vkDevice, Image->Image, Image->Memory, 0);
	hard_assert_eq(Result, VK_SUCCESS);

	VkImageViewCreateInfo ViewInfo = {0};
	ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ViewInfo.pNext = NULL;
	ViewInfo.flags = 0;
	ViewInfo.image = Image->Image;
	ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	ViewInfo.format = Image->format;
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
	hard_assert_eq(Result, VK_SUCCESS);

	if(Image->type == IMAGE_TYPE_TEXTURE)
	{
		VulkanTransitionImageLayout(Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VulkanCopyTextureToImage(Image, tex);

		dds_free(tex);

		VulkanTransitionImageLayout(Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}


private void
VulkanDestroyImage(
	Image* Image
	)
{
	vkFreeMemory(vkDevice, Image->Memory, NULL);
	vkDestroyImageView(vkDevice, Image->View, NULL);
	vkDestroyImage(vkDevice, Image->Image, NULL);
}


private void
VulkanInitImages(
	void
	)
{
	VulkanCreateImage(&vkDepthBuffer);
	VulkanCreateImage(&vkMultisampling);
}


private void
VulkanDestroyImages(
	void
	)
{
	VulkanDestroyImage(&vkMultisampling);
	VulkanDestroyImage(&vkDepthBuffer);
}


private VkShaderModule
VulkanCreateShader(
	const char* path
	)
{
	file_t file;
	bool status = file_read(path, &file);
	hard_assert_true(status);

	const uint32_t* Code = (const uint32_t*) file.data;

	VkShaderModuleCreateInfo ShaderInfo = {0};
	ShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderInfo.pNext = NULL;
	ShaderInfo.flags = 0;
	ShaderInfo.codeSize = file.len;
	ShaderInfo.pCode = Code;

	VkShaderModule Shader;

	VkResult Result = vkCreateShaderModule(vkDevice, &ShaderInfo, NULL, &Shader);
	hard_assert_eq(Result, VK_SUCCESS);

	file_free(file);

	return Shader;
}


private void
VulkanDestroyShader(
	VkShaderModule Shader
	)
{
	vkDestroyShaderModule(vkDevice, Shader, NULL);
}


private void
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
	Attachments[0].format = vkMultisampling.format;
	Attachments[0].samples = vkSamples;
	Attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	Attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	Attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	Attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	Attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	Attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	Attachments[1].flags = 0;
	Attachments[1].format = vkDepthBuffer.format;
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
	RenderPassInfo.attachmentCount = MACRO_ARRAY_LEN(Attachments);
	RenderPassInfo.pAttachments = Attachments;
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &Subpass;
	RenderPassInfo.dependencyCount = 1;
	RenderPassInfo.pDependencies = &Dependency;

	VkResult Result = vkCreateRenderPass(vkDevice, &RenderPassInfo, NULL, &vkRenderPass);
	hard_assert_eq(Result, VK_SUCCESS);


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
	DynamicState.dynamicStateCount = MACRO_ARRAY_LEN(DynamicStates);
	DynamicState.pDynamicStates = DynamicStates;

	VkVertexInputBindingDescription VertexBindings[2] = {0};

	VertexBindings[0].binding = 0;
	VertexBindings[0].stride = sizeof(VkVertexVertexInput);
	VertexBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VertexBindings[1].binding = 1;
	VertexBindings[1].stride = sizeof(window_draw_data_t);
	VertexBindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	VkVertexInputAttributeDescription Attributes[] =
	{
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(VkVertexVertexInput, pos)
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
			.offset = offsetof(window_draw_data_t, pos)
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(window_draw_data_t, size)
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.offset = offsetof(window_draw_data_t, white_color)
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(window_draw_data_t, white_depth)
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.offset = offsetof(window_draw_data_t, black_color)
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(window_draw_data_t, black_depth)
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R16G16_UINT,
			.offset = offsetof(window_draw_data_t, tex)
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(window_draw_data_t, angle)
		},
		{
			.location = 10,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(window_draw_data_t, tex_scale)
		},
		{
			.location = 11,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(window_draw_data_t, tex_offset)
		}
	};

	VkPipelineVertexInputStateCreateInfo VertexInput = {0};
	VertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInput.pNext = NULL;
	VertexInput.flags = 0;
	VertexInput.vertexBindingDescriptionCount = MACRO_ARRAY_LEN(VertexBindings);
	VertexInput.pVertexBindingDescriptions = VertexBindings;
	VertexInput.vertexAttributeDescriptionCount = MACRO_ARRAY_LEN(Attributes);
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
	Bindings[0].descriptorCount = MACRO_ARRAY_LEN(vkTextures);
	Bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	Bindings[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo Descriptors = {0};
	Descriptors.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	Descriptors.pNext = NULL;
	Descriptors.flags = 0;
	Descriptors.bindingCount = MACRO_ARRAY_LEN(Bindings);
	Descriptors.pBindings = Bindings;

	Result = vkCreateDescriptorSetLayout(vkDevice, &Descriptors, NULL, &vkDescriptors);
	hard_assert_eq(Result, VK_SUCCESS);

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
	LayoutInfo.pushConstantRangeCount = MACRO_ARRAY_LEN(PushConstants);
	LayoutInfo.pPushConstantRanges = PushConstants;

	Result = vkCreatePipelineLayout(vkDevice, &LayoutInfo, NULL, &vkPipelineLayout);
	hard_assert_eq(Result, VK_SUCCESS);


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
	hard_assert_eq(Result, VK_SUCCESS);

	VulkanDestroyShader(VertexModule);
	VulkanDestroyShader(FragmentModule);
}


private void
VulkanDestroyPipeline(
	void
	)
{
	vkDestroyPipeline(vkDevice, vkPipeline, NULL);
	vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
	vkDestroyDescriptorSetLayout(vkDevice, vkDescriptors, NULL);
	vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
}


private void
VulkanInitConsts(
	void
	)
{
	VkBarrier* Barrier = vkBarriers;
	VkBarrier* BarrierEnd = Barrier + MACRO_ARRAY_LEN(vkBarriers);

	do
	{
		VkSemaphoreCreateInfo SemaphoreInfo = {0};
		SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		SemaphoreInfo.pNext = NULL;
		SemaphoreInfo.flags = 0;

		VkSemaphore* sync_sem_t = Barrier->Semaphores;
		VkSemaphore* SemaphoreEnd = sync_sem_t + MACRO_ARRAY_LEN(Barrier->Semaphores);

		do
		{
			VkResult Result = vkCreateSemaphore(vkDevice, &SemaphoreInfo, NULL, sync_sem_t);
			hard_assert_eq(Result, VK_SUCCESS);
		}
		while(++sync_sem_t != SemaphoreEnd);


		VkFenceCreateInfo FenceInfo = {0};
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.pNext = NULL;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence* Fence = Barrier->Fences;
		VkFence* FenceEnd = Barrier->Fences + MACRO_ARRAY_LEN(Barrier->Fences);

		do
		{
			VkResult Result = vkCreateFence(vkDevice, &FenceInfo, NULL, Fence);
			hard_assert_eq(Result, VK_SUCCESS);
		}
		while(++Fence != FenceEnd);
	}
	while(++Barrier != BarrierEnd);


	VkCommandBuffer CommandBuffers[MACRO_ARRAY_LEN(vkFrames)];

	VkCommandBufferAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.commandPool = vkCommandPool;
	AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	AllocInfo.commandBufferCount = MACRO_ARRAY_LEN(CommandBuffers);

	VkResult Result = vkAllocateCommandBuffers(vkDevice, &AllocInfo, CommandBuffers);
	hard_assert_eq(Result, VK_SUCCESS);


	VkFrame* Frame = vkFrames;
	VkFrame* FrameEnd = Frame + MACRO_ARRAY_LEN(vkFrames);

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


private void
VulkanDestroyConsts(
	void
	)
{
	VkFrame* Frame = vkFrames;
	VkFrame* FrameEnd = Frame + MACRO_ARRAY_LEN(vkFrames);

	VkCommandBuffer CommandBuffers[MACRO_ARRAY_LEN(vkFrames)];
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

	vkFreeCommandBuffers(vkDevice, vkCommandPool, MACRO_ARRAY_LEN(CommandBuffers), CommandBuffers);


	VkBarrier* Barrier = vkBarriers;
	VkBarrier* BarrierEnd = Barrier + MACRO_ARRAY_LEN(vkBarriers);

	do
	{
		VkFence* Fence = Barrier->Fences;
		VkFence* FenceEnd = Barrier->Fences + MACRO_ARRAY_LEN(Barrier->Fences);

		do
		{
			vkDestroyFence(vkDevice, *Fence, NULL);
		}
		while(++Fence != FenceEnd);


		VkSemaphore* sync_sem_t = Barrier->Semaphores;
		VkSemaphore* SemaphoreEnd = Barrier->Semaphores + MACRO_ARRAY_LEN(Barrier->Semaphores);

		do
		{
			vkDestroySemaphore(vkDevice, *sync_sem_t, NULL);
		}
		while(++sync_sem_t != SemaphoreEnd);
	}
	while(++Barrier != BarrierEnd);
}


private void
VulkanInitFrames(
	void
	)
{
	uint32_t ImageCount;
	VkResult Result = vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &ImageCount, NULL);
	hard_assert_eq(Result, VK_SUCCESS);

	if(vkImageCount == 0)
	{
		assert_neq(ImageCount, 0);
		assert_le(ImageCount, MACRO_ARRAY_LEN(vkFrames));
		vkImageCount = ImageCount;
	}
	else
	{
		assert_eq(ImageCount, vkImageCount);
	}

	VkImage Images[vkImageCount];
	Result = vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkImageCount, Images);
	hard_assert_eq(Result, VK_SUCCESS);


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
		hard_assert_eq(Result, VK_SUCCESS);


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
		FramebufferInfo.attachmentCount = MACRO_ARRAY_LEN(Attachments);
		FramebufferInfo.pAttachments = Attachments;
		FramebufferInfo.width = vkExtent.width;
		FramebufferInfo.height = vkExtent.height;
		FramebufferInfo.layers = 1;

		Result = vkCreateFramebuffer(vkDevice, &FramebufferInfo, NULL, &Frame->Framebuffer);
		hard_assert_eq(Result, VK_SUCCESS);


		if(++Frame == FrameEnd)
		{
			break;
		}

		++Image;
	}
}


private void
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


private VkSwapchainKHR
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
	hard_assert_eq(Result, VK_SUCCESS);

	return Swapchain;
}


private void
VulkanInitSwapchain(
	void
	)
{
	vkSwapchain = VulkanCreateSwapchain(VK_NULL_HANDLE);

	VulkanInitFrames();
}


private void
VulkanDestroySwapchain(
	void
	)
{
	VulkanDestroyFrames();

	vkDestroySwapchainKHR(vkDevice, vkSwapchain, NULL);
}


private void
VulkanReinitSwapchain(
	void
	)
{
	VkSwapchainKHR NewSwapchain = VulkanCreateSwapchain(vkSwapchain);

	VulkanDestroySwapchain();

	vkSwapchain = NewSwapchain;

	VulkanInitFrames();
}


private void
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
	hard_assert_eq(Result, VK_SUCCESS);


	Image* tex = vkTextures;
	Image* TextureEnd = vkTextures + MACRO_ARRAY_LEN(vkTextures);

	const tex_file_t* TextureFile = TextureFiles;

	while(tex != TextureEnd)
	{
		tex->path = TextureFile->path;
		tex->format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		tex->type = IMAGE_TYPE_TEXTURE;

		VulkanCreateImage(tex);

		++TextureFile;
		++tex;
	}


	VkDescriptorPoolSize PoolSizes[1] = {0};

	PoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	PoolSizes[0].descriptorCount = MACRO_ARRAY_LEN(vkTextures);

	VkDescriptorPoolCreateInfo DescriptorInfo = {0};
	DescriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorInfo.pNext = NULL;
	DescriptorInfo.flags = 0;
	DescriptorInfo.maxSets = 1;
	DescriptorInfo.poolSizeCount = MACRO_ARRAY_LEN(PoolSizes);
	DescriptorInfo.pPoolSizes = PoolSizes;

	Result = vkCreateDescriptorPool(vkDevice, &DescriptorInfo, NULL, &vkDescriptorPool);
	hard_assert_eq(Result, VK_SUCCESS);


	VkDescriptorSetAllocateInfo AllocInfo = {0};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.pNext = NULL;
	AllocInfo.descriptorPool = vkDescriptorPool;
	AllocInfo.descriptorSetCount = 1;
	AllocInfo.pSetLayouts = &vkDescriptors;

	Result = vkAllocateDescriptorSets(vkDevice, &AllocInfo, &vkDescriptorSet);
	hard_assert_eq(Result, VK_SUCCESS);


	VkDescriptorImageInfo ImageInfos[MACRO_ARRAY_LEN(vkTextures)] = {0};
	VkWriteDescriptorSet DescriptorWrites[MACRO_ARRAY_LEN(vkTextures)] = {0};

	tex = vkTextures;

	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(vkTextures); ++i) {
		ImageInfos[i].sampler = vkSampler;
		ImageInfos[i].imageView = tex->View;
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

		++tex;
	}

	vkUpdateDescriptorSets(vkDevice, MACRO_ARRAY_LEN(vkTextures), DescriptorWrites, 0, NULL);
}


private void
VulkanDestroyTextures(
	void
	)
{
	vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);


	Image* tex = vkTextures;
	Image* TextureEnd = vkTextures + MACRO_ARRAY_LEN(vkTextures);

	while(tex != TextureEnd)
	{
		VulkanDestroyImage(tex);

		++tex;
	}

	vkDestroySampler(vkDevice, vkSampler, NULL);
}


private void
VulkanInitVertex(
	void
	)
{
	VulkanGetVertexBuffer(sizeof(vkVertexVertexInput), &vkVertexVertexInputBuffer, &vkVertexVertexInputMemory);
	VulkanCopyToBuffer(vkVertexVertexInputBuffer, vkVertexVertexInput, sizeof(vkVertexVertexInput));

	VulkanGetVertexBuffer(sizeof(window_draw_data_t) * GAME_CONST_MAX_TEXTURES,
		&vkVertexInstanceInputBuffer, &vkVertexInstanceInputMemory);

	DrawDataBuffer = alloc_malloc(sizeof(window_draw_event_data_t) * GAME_CONST_MAX_TEXTURES);
	hard_assert_not_null(DrawDataBuffer);

	VulkanGetDrawIndirectBuffer(sizeof(VkDrawIndirectCommand), &vkDrawCountBuffer, &vkDrawCountMemory);
}


private void
VulkanDestroyVertex(
	void
	)
{
	vkFreeMemory(vkDevice, vkDrawCountMemory, NULL);
	vkDestroyBuffer(vkDevice, vkDrawCountBuffer, NULL);

	alloc_free(sizeof(window_draw_event_data_t) * GAME_CONST_MAX_TEXTURES, DrawDataBuffer);

	vkFreeMemory(vkDevice, vkVertexInstanceInputMemory, NULL);
	vkDestroyBuffer(vkDevice, vkVertexInstanceInputBuffer, NULL);

	vkFreeMemory(vkDevice, vkVertexVertexInputMemory, NULL);
	vkDestroyBuffer(vkDevice, vkVertexVertexInputBuffer, NULL);
}


private void
VulkanInitSync(
	void
	)
{
	sync_mtx_init(&mtx);
	sync_cond_init(&Cond);
	sync_mtx_init(&WindowMtx);
}


private void
VulkanDestroySync(
	void
	)
{
	sync_mtx_free(&WindowMtx);
	sync_cond_free(&Cond);
	sync_mtx_free(&mtx);
}


private void
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
		hard_assert_eq(Result, VK_SUCCESS);

		VkCommandBufferBeginInfo BeginInfo = {0};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.pNext = NULL;
		BeginInfo.flags = 0;
		BeginInfo.pInheritanceInfo = NULL;

		Result = vkBeginCommandBuffer(Frame->CommandBuffer, &BeginInfo);
		hard_assert_eq(Result, VK_SUCCESS);

		VkRenderPassBeginInfo RenderPassInfo = {0};
		RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPassInfo.pNext = NULL;
		RenderPassInfo.renderPass = vkRenderPass;
		RenderPassInfo.framebuffer = Frame->Framebuffer;
		RenderPassInfo.renderArea.offset.x = 0;
		RenderPassInfo.renderArea.offset.y = 0;
		RenderPassInfo.renderArea.extent = vkExtent;
		RenderPassInfo.clearValueCount = MACRO_ARRAY_LEN(ClearValues);
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
		hard_assert_eq(Result, VK_SUCCESS);
	}
	while(++Frame != FrameEnd);
}


private void
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
window_add_draw_event_data(
	const window_draw_event_data_t* data
	)
{
	DrawDataBuffer[DrawDataCount++] = *data;
}


private void
VulkanDraw(
	void
	)
{
	VkResult Result = vkWaitForFences(vkDevice, 1, vkBarrier->Fences + BARRIER_FENCE_IN_FLIGHT, VK_TRUE, UINT64_MAX);
	hard_assert_eq(Result, VK_SUCCESS);

	uint32_t ImageIndex;
	Result = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX,
		vkBarrier->Semaphores[BARRIER_SEMAPHORE_IMAGE_AVAILABLE], VK_NULL_HANDLE, &ImageIndex);
	if(Result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		VulkanRecreateSwapchain();

		return;
	}

	VkFrame* Frame = vkFrames + ImageIndex;

	Result = vkResetFences(vkDevice, 1, vkBarrier->Fences + BARRIER_FENCE_IN_FLIGHT);
	hard_assert_eq(Result, VK_SUCCESS);


	DrawDataCount = 0;
	sync_mtx_lock(&WindowMtx);
		event_target_fire(&window_draw_target, &((window_draw_event_data_t){0}));
	sync_mtx_unlock(&WindowMtx);

	uint32_t total_size = sizeof(window_draw_event_data_t) * DrawDataCount;
	VulkanCopyToBuffer(vkVertexInstanceInputBuffer, DrawDataBuffer, total_size);


	VkDrawIndirectCommand Command = {0};
	Command.vertexCount = MACRO_ARRAY_LEN(vkVertexVertexInput);
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
	SubmitInfo.pWaitSemaphores = vkBarrier->Semaphores + BARRIER_SEMAPHORE_IMAGE_AVAILABLE;
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &Frame->CommandBuffer;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = vkBarrier->Semaphores + BARRIER_SEMAPHORE_RENDER_FINISHED;

	Result = vkQueueSubmit(vkQueue, 1, &SubmitInfo, vkBarrier->Fences[BARRIER_FENCE_IN_FLIGHT]);
	hard_assert_eq(Result, VK_SUCCESS);

	VkPresentInfoKHR PresentInfo = {0};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = NULL;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = vkBarrier->Semaphores + BARRIER_SEMAPHORE_RENDER_FINISHED;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &vkSwapchain;
	PresentInfo.pImageIndices = &ImageIndex;
	PresentInfo.pResults = NULL;

	Result = vkQueuePresentKHR(vkQueue, &PresentInfo);

	if(Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
	{
		VulkanRecreateSwapchain();
	}

	if(++vkBarrier == vkBarriers + MACRO_ARRAY_LEN(vkBarriers))
	{
		vkBarrier = vkBarriers;
	}
}


void
window_init(
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


private void
VulkanThreadFN(
	void* data
	)
{
	(void) data;

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


private void
WindowInterrupt(
	int Signal
	)
{
	WindowQuit();
}


void
window_run(
	void
	)
{
	signal(SIGINT, WindowInterrupt);

	atomic_init(&vkShouldRun, 1);

	thread_data_t data =
	(thread_data_t)
	{
		.fn = VulkanThreadFN,
		.data = NULL
	};

	thread_init(&vkThread, data);

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
window_free(
	void
	)
{
	thread_join(vkThread);
	thread_free(&vkThread);

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
