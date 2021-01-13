#ifndef LVK_DEVICE_SELECTOR_H
#define LVK_DEVICE_SELECTOR_H

#include <vector>
#include <vulkan/vulkan.h>
#include <functional>
#include "device.h"

namespace lvk
{
    inline namespace literals
    {
        constexpr uint64_t operator""_GB(unsigned long long const x)
        {
            return 1024 * 1024 * 1024 * x;
        }
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

    class device_selector
    {
    public:
        device_selector(VkInstance instance);

        device_selector & prefer_device_type(device_preference type_preference);
        device_selector & require_compute_queue();
        device_selector & require_separate_compute_queue();
        device_selector & require_exclusive_compute_queue();
        device_selector & require_transfer_queue();
        device_selector & require_separate_transfer_queue();
        device_selector & require_exclusive_transfer_queue();
        device_selector & require_present_support();
        device_selector & with_extension(const char * name);
        device_selector & with_extensions(const std::vector<const char *> & names);
        device_selector & minimum_memory(VkDeviceSize size);
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

        enum class queue_preference
        {
            any,
            available,
            separate_from_graphics,
            exclusive
        };
        struct device_criteria
        {
            device_preference type_preference = device_preference::any;
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