#ifndef LVK_DEVICE_SELECTOR_H
#define LVK_DEVICE_SELECTOR_H

#include <vector>
#include <vulkan/vulkan.h>
#include <functional>

namespace lvk
{
    constexpr uint64_t operator""_GoB(unsigned long long const x)
    {
        return 1024 * 1024 * 1024 * x;
    }
    struct physical_device
    {
        physical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface = VK_NULL_HANDLE);

        int32_t find_queue_family(VkQueueFlags flags) const;
        int32_t find_exclusive_queue_family(VkQueueFlags flags) const;
        int32_t find_present_supported_queue_family() const;
        VkBool32 queue_family_supports_present(uint32_t index) const;
        bool supports_features(VkPhysicalDeviceFeatures requested_features) const;

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

    enum class device_preference
    {
        other = VK_PHYSICAL_DEVICE_TYPE_OTHER,
        integrated_gpu = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
        discrete_gpu = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        virtual_gpu = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
        cpu = VK_PHYSICAL_DEVICE_TYPE_CPU,
        any = VK_PHYSICAL_DEVICE_TYPE_END_RANGE + 1
    };
    enum class queue_preference
    {
        any,
        supported,
        dedicated
    };

    class device_selector
    {
    public:
        device_selector(VkInstance instance, VkSurfaceKHR surface = VK_NULL_HANDLE);

        device_selector & min_api_version(uint32_t api);
        device_selector & min_api_version(uint32_t major, uint32_t minor, uint32_t patch);

        device_selector & preset_graphics(VkDeviceSize minimum_memory = 1_GoB);

        physical_device select();
    private:
        VkInstance vk_instance;
        VkSurfaceKHR surface;
        std::vector<VkPhysicalDevice> physical_devices;

        struct device_criteria
        {
            device_preference device_preference = device_preference::any;
            queue_preference graphics_queue_preference = queue_preference::any;
            queue_preference compute_queue_preference = queue_preference::any;
            queue_preference transfer_queue_preference = queue_preference::any;
            bool present_support = false;
            std::vector<const char *> extensions;
            VkDeviceSize available_memory = 0;
            uint32_t vulkan_api_version = VK_MAKE_VERSION(1, 0, 0);
            VkPhysicalDeviceFeatures features{};
        } criteria;
        bool device_passes_criteria(const physical_device & candidate);
    };
}

#endif