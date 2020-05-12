#ifndef LVK_H
#define LVK_H

#include <vulkan/vulkan.h>
#include <SDL/SDL_vulkan.h>
#include <vector>

namespace lvk
{
    VkApplicationInfo make_application_info(const char * app_title, uint32_t app_version, uint32_t api_version);
    VkInstanceCreateInfo make_instance_create_info(VkApplicationInfo * app_info, const std::vector<const char *> & extensions, const std::vector<const char *> & validation_layers);
    VkDebugUtilsMessengerEXT make_default_debug_messenger(VkInstance instance, const VkAllocationCallbacks * allocator = nullptr);
    void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator = nullptr);
}

#endif