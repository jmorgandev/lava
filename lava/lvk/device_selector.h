#ifndef LVK_DEVICE_SELECTOR_H
#define LVK_DEVICE_SELECTOR_H

#include <vector>
#include <vulkan/vulkan.h>
#include <functional>
#include "device.h"

namespace lvk
{
    constexpr uint64_t operator""_GB(unsigned long long const x)
    {
        return 1024 * 1024 * 1024 * x;
    }
    

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
        device_selector(VkInstance instance);

        device_selector & render_surface(VkSurfaceKHR);
        device_selector & min_api_version(uint32_t api);
        device_selector & min_api_version(uint32_t major, uint32_t minor, uint32_t patch);
        device_selector & with_features(VkPhysicalDeviceFeatures features);

        device_selector & preset_graphics(VkDeviceSize minimum_memory = 1_GB);

        physical_device select();
    private:
        VkInstance vk_instance;
        VkSurfaceKHR vk_surface;
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