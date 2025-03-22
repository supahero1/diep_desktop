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
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/client/tex/base.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/client/window/graphics.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <DiepDesktop/client/window/volk.h>

#define VULKAN_MAX_IMAGES 8


typedef struct graphics_frame
{
	VkImage image;
	VkImageView view;
	VkFramebuffer frame_buffer;
	VkCommandBuffer command_buffer;
}
graphics_frame_t;

typedef enum graphics_barrier_semaphore
{
	GRAPHICS_BARRIER_SEMAPHORE_IMAGE_AVAILABLE,
	GRAPHICS_BARRIER_SEMAPHORE_RENDER_FINISHED,
	MACRO_ENUM_BITS(GRAPHICS_BARRIER_SEMAPHORE)
}
graphics_barrier_semaphore_t;

typedef enum graphics_barrier_fence
{
	GRAPHICS_BARRIER_FENCE_IN_FLIGHT,
	MACRO_ENUM_BITS(GRAPHICS_BARRIER_FENCE)
}
graphics_barrier_fence_t;

typedef struct graphics_barrier
{
	VkSemaphore semaphores[GRAPHICS_BARRIER_SEMAPHORE__COUNT];
	VkFence fences[GRAPHICS_BARRIER_FENCE__COUNT];
}
graphics_barrier_t;

typedef enum graphics_image_type
{
	GRAPHICS_IMAGE_TYPE_DEPTH_STENCIL,
	GRAPHICS_IMAGE_TYPE_MULTISAMPLED,
	GRAPHICS_IMAGE_TYPE_TEXTURE,
	MACRO_ENUM_BITS(GRAPHICS_IMAGE_TYPE)
}
graphics_image_type_t;

typedef struct graphics_image
{
	const char* path;

	uint32_t width;
	uint32_t height;
	uint32_t layers;

	VkFormat format;
	graphics_image_type_t type;

	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;

	VkImageAspectFlags aspect;
	VkImageUsageFlags usage;
	VkSampleCountFlagBits samples;
}
graphics_image_t;

typedef struct graphics_vertex_input
{
	pair_t pos;
	pair_t tex_coord;
}
graphics_vertex_input_t;

typedef struct graphics_vertex_consts
{
	pair_t window_size;
}
graphics_vertex_consts_t;

struct graphics_impl
{
	window_t* window;

	uint32_t frame_number;

	struct VolkDeviceTable table;

	VkDebugUtilsMessengerEXT messenger;

	VkInstance instance;
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surface_capabilities;

	uint32_t queue_id;
	VkSampleCountFlagBits samples;
	VkPhysicalDeviceLimits device_limits;

	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue queue;
	VkPhysicalDeviceMemoryProperties memory_properties;

	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
	VkFence fence;

	VkExtent2D extent;
	uint32_t min_image_count;
	VkSurfaceTransformFlagBitsKHR transform;
	VkPresentModeKHR present_mode;

	VkSwapchainKHR swapchain;
	uint32_t image_count;
	graphics_frame_t frames[VULKAN_MAX_IMAGES];

	graphics_barrier_t barriers[VULKAN_MAX_IMAGES];
	graphics_barrier_t* barrier;

	VkViewport viewport;
	VkRect2D scissor;

	VkBuffer copy_buffer;
	VkDeviceMemory copy_buffer_memory;

	VkSampler sampler;

	graphics_image_t depth_buffer;
	graphics_image_t multisampling_buffer;

	graphics_image_t textures[TEX__COUNT];

	VkBuffer vertex_input_buffer;
	VkDeviceMemory vertex_input_memory;

	VkBuffer instance_input_buffer;
	VkDeviceMemory instance_input_memory;

	VkBuffer draw_count_buffer;
	VkDeviceMemory draw_count_memory;

	graphics_vertex_consts_t consts;

	VkDescriptorSetLayout descriptors;
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_set;

	graphics_draw_data_t* draw_data_buffer;
	uint32_t draw_data_used;
	uint32_t draw_data_size;

	thread_t thread;
	_Atomic uint8_t should_run;

	sync_mtx_t resize_mtx;
	sync_cond_t resize_cond;
	bool resized;

	sync_mtx_t window_mtx;
};


private const graphics_vertex_input_t graphics_vertex_input[] =
{
	{ {{ -0.5f, -0.5f }}, {{ 0.0f, 0.0f }} },
	{ {{  0.5f, -0.5f }}, {{ 1.0f, 0.0f }} },
	{ {{ -0.5f,  0.5f }}, {{ 0.0f, 1.0f }} },
	{ {{  0.5f,  0.5f }}, {{ 1.0f, 1.0f }} },
};


private void
graphics_free(
	graphics_t* graphics,
	window_free_event_data_t* event_data
	)
{
	assert_not_null(graphics);

	event_target_free(&graphics->draw_target);

	alloc_free(graphics->impl, sizeof(*graphics->impl));
}


void
graphics_init(
	graphics_t* graphics,
	window_t* window
	)
{
	assert_not_null(graphics);
	assert_not_null(window);

	graphics->impl = alloc_malloc(sizeof(*graphics->impl));
	assert_ptr(graphics->impl, sizeof(*graphics->impl));

	graphics->impl->window = window;

	event_listener_data_t free_data =
	{
		.fn = (event_fn_t) graphics_free,
		.data = graphics
	};
	(void) event_target_once(&window->free_target, free_data);

	event_target_init(&graphics->draw_target);
}


static void
graphics_resize_draw_data(
	graphics_t* graphics,
	uint32_t count
	)
{
	uint32_t new_used = graphics->impl->draw_data_used;

	if((new_used < (graphics->impl->draw_data_size >> 2)) || (new_used > graphics->impl->draw_data_size))
	{
		uint32_t new_size = (new_used << 1) | 1;

		graphics->impl->draw_data_buffer = alloc_remalloc(
			graphics->impl->draw_data_buffer,
			sizeof(*graphics->impl->draw_data_buffer) * graphics->impl->draw_data_size,
			sizeof(*graphics->impl->draw_data_buffer) * new_size
			);
		assert_not_null(graphics->impl->draw_data_buffer);

		graphics->impl->draw_data_size = new_size;
	}
}
