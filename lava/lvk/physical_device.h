#ifndef LVK_PHYSICAL_DEVICE_H
#define LVK_PHYSICAL_DEVICE_H

#include <vulkan/vulkan.h>
#include <vector>

#include "object.h"

namespace lvk
{
    struct surface_details
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    class physical_device : public object<VkPhysicalDevice>
    {
        friend class device_selector;
        friend class swapchain_builder;
    public:
        physical_device() = default;
        physical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface = VK_NULL_HANDLE);

        bool has_compatible_queue_family(VkQueueFlags flags) const;
        bool has_mutually_exclusive_queue_family(VkQueueFlags flags, VkQueueFlags exclude_flags) const;
        bool has_exclusive_queue_family(VkQueueFlags flags) const;
        bool has_present_supported_queue_family() const;
        VkBool32 queue_family_supports_present(uint32_t index) const;
        uint32_t compatible_queue_family_index(VkQueueFlags flags) const;
        uint32_t mutually_exclusive_queue_family_index(VkQueueFlags flags, VkQueueFlags exclude_flags) const;
        uint32_t exclusive_queue_family_index(VkQueueFlags flags) const;
        uint32_t present_queue_family_index() const;

        bool supports_features(VkPhysicalDeviceFeatures requested_features) const;
        VkSampleCountFlagBits max_usable_sample_count() const;

        VkExtent2D choose_swapchain_extent(uint32_t width, uint32_t height) const;
        VkSurfaceFormatKHR choose_swapchain_surface_format() const;
        VkPresentModeKHR choose_swapchain_present_mode() const;
        uint32_t choose_swapchain_length() const;

        const surface_details query_surface_details() const;
    private:
        VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties properties = {};
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        VkPhysicalDeviceFeatures features = {};
        VkDeviceSize local_memory_size = 0;
        uint32_t api_version = 0;
        std::vector<VkQueueFamilyProperties> queue_families;
        std::vector<VkExtensionProperties> extensions;
    };
}

#endif