#ifndef LVK_H
#define LVK_H

#include <vulkan/vulkan.h>
#include <SDL/SDL_vulkan.h>
#include <vector>

namespace lvk
{
    VkDebugUtilsMessengerCreateInfoEXT make_default_debug_messenger_create_info();
    VkDebugUtilsMessengerEXT make_debug_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * create_info, const VkAllocationCallbacks * allocator = nullptr);
    void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator = nullptr);
    std::vector<const char *> filter_supported_extensions(std::vector<const char *> requested_extensions);
    std::vector<const char *> filter_supported_layers(std::vector<const char *> requested_layers);
}

#endif