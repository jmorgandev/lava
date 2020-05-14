#ifndef LVK_H
#define LVK_H

#include <vulkan/vulkan.h>
#include <SDL/SDL_vulkan.h>
#include <vector>

#define LVK_NULL_QUEUE_FAMILY -1

namespace lvk
{
    VkDebugUtilsMessengerCreateInfoEXT make_default_debug_messenger_create_info();
    VkDebugUtilsMessengerEXT make_debug_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * create_info, const VkAllocationCallbacks * allocator = nullptr);
    void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator = nullptr);
    std::vector<const char *> filter_supported_extensions(std::vector<const char *> requested_extensions);
    std::vector<const char *> filter_supported_layers(std::vector<const char *> requested_layers);

    struct QueueFamilyInfo
    {
        int graphics_queue = LVK_NULL_QUEUE_FAMILY;
    };
    QueueFamilyInfo get_queue_family_info(VkPhysicalDevice device);
}

#endif