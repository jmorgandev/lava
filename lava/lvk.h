#ifndef LVK_H
#define LVK_H

#include <vulkan/vulkan.h>
#include <SDL/SDL_vulkan.h>
#include <vector>

#define LVK_NULL_QUEUE_FAMILY -1
#define USE_VALIDATION _DEBUG || true
#define DEVICE_VALIDATION_LAYER_COMPATIBILITY false

namespace lvk
{
    VkDebugUtilsMessengerCreateInfoEXT make_default_debug_messenger_create_info();
    VkDebugUtilsMessengerEXT make_debug_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * create_info, const VkAllocationCallbacks * allocator = nullptr);
    void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator = nullptr);
    std::vector<const char *> filter_supported_extensions(std::vector<const char *> requested_extensions);
    std::vector<const char *> filter_supported_layers(std::vector<const char *> requested_layers);
    std::vector<const char *> filter_supported_device_extensions(VkPhysicalDevice device, std::vector<const char *> requested_extensions);

    struct QueueFamilyInfo
    {
        int graphics_family = LVK_NULL_QUEUE_FAMILY;
        int present_family = LVK_NULL_QUEUE_FAMILY;
    };
    QueueFamilyInfo get_queue_family_info(VkPhysicalDevice device, VkSurfaceKHR surface);
}

#endif