#ifndef LAVA_RENDERER_H
#define LAVA_RENDERER_H

#include <vulkan/vulkan.h>

class lava_app;

class lava_renderer
{
public:
    lava_renderer(lava_app * app);
    ~lava_renderer();

    VkInstance vulkan_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSurfaceKHR window_surface;
    VkSwapchainKHR swapchain;
};

#endif