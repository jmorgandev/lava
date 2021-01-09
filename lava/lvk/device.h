#ifndef LVK_DEVICE_H
#define LVK_DEVICE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace lvk
{
    class physical_device
    {
        friend class device_selector;
    public:
        physical_device();
        physical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface = VK_NULL_HANDLE);

        bool has_compatible_queue_family(VkQueueFlags flags) const;
        bool has_exclusive_queue_family(VkQueueFlags flags) const;
        bool has_present_supported_queue_family() const;
        VkBool32 queue_family_supports_present(uint32_t index) const;
        bool supports_features(VkPhysicalDeviceFeatures requested_features) const;

        VkPhysicalDevice vk() const { return vk_physical_device; }
    private:
        VkPhysicalDevice vk_physical_device;
        VkSurfaceKHR surface;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memory_properties;
        VkPhysicalDeviceFeatures features;
        VkDeviceSize local_memory_size;
        uint32_t api_version;
        std::vector<VkQueueFamilyProperties> queue_families;
        std::vector<VkExtensionProperties> extensions;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> present_modes;
    };
}

#endif