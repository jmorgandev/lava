#ifndef LVK_H
#define LVK_H

#include <vulkan/vulkan.h>
#include <SDL/SDL_vulkan.h>
#include <vector>

constexpr int LVK_NULL_QUEUE_FAMILY = -1;
#define USE_VALIDATION _DEBUG || true
#define DEVICE_VALIDATION_LAYER_COMPATIBILITY false

namespace lvk
{
    void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator = nullptr);
    std::vector<const char *> filter_supported_extensions(std::vector<const char *> requested_extensions);
    std::vector<const char *> filter_supported_layers(std::vector<const char *> requested_layers);
    std::vector<const char *> filter_supported_device_extensions(VkPhysicalDevice device, std::vector<const char *> requested_extensions);

    struct QueueFamilyInfo
    {
        int graphics_family;
        int present_family;
    };
    QueueFamilyInfo get_queue_family_info(VkPhysicalDevice device, VkSurfaceKHR surface);

    struct DeviceSurfaceDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };
    DeviceSurfaceDetails query_surface_details(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkSurfaceFormatKHR choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> & available_formats);
    VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> & available_modes);
    VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR & capabilities, uint32_t window_width, uint32_t window_height);

    VkShaderModule create_shader_module(VkDevice device, const std::vector<char> & source);

    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice device);

    ///////////// START OF REFACTOR

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT * callback_data, void * user_data);
    VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_create_info();

    VkInstance create_instance(uint32_t api_version, std::vector<const char *> extensions, std::vector<const char *> layers, const void * pNext = nullptr, const char * app_name = "Lava Application");
    VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT * create_info = nullptr);

    struct PhysicalDeviceDetails
    {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceMemoryProperties memory_properties;
        std::vector<VkQueueFamilyProperties> queue_families;
        std::vector<VkExtensionProperties> extensions;
        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> present_modes;

        bool has_graphics_queue;
        bool has_transfer_queue;
        bool has_swapchain_extension;

        VkDeviceSize local_memory_size;
        VkDeviceSize total_memory_size;
    };
    PhysicalDeviceDetails get_physical_device_details(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);
}

#endif