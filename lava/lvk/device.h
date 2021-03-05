#ifndef LVK_DEVICE_H
#define LVK_DEVICE_H

#include <vulkan/vulkan.h>
#include <vector>
#include "physical_device.h"
#include "queue.h"

namespace lvk
{
    class device
    {
    public:
        device(VkDevice device = VK_NULL_HANDLE, std::vector<queue> queues = {});
        void destroy();
        ~device() {}

        const std::vector<queue> & queues() { return active_queues; }
        VkDevice vk() const { return vk_device; }
    private:
        VkDevice vk_device;
        std::vector<queue> active_queues;
    };

    class device_builder
    {
    public:
        device_builder(const physical_device & physical_device);

        device_builder & queues(uint32_t family_index, uint32_t count, std::vector<float> priorities);
        device_builder & queues(uint32_t family_index, uint32_t count);
        device_builder & extension(const char * name);
        device_builder & extensions(std::vector<const char *> names);
        device_builder & features(VkPhysicalDeviceFeatures features);

        device build();
    private:
        const physical_device & phys_device;
        std::pair<std::vector<VkDeviceQueueCreateInfo>, std::vector<std::vector<float>>> queue_infos_and_priorities;
        std::vector<const char *> enabled_extensions;
        VkPhysicalDeviceFeatures enabled_features;
    };
}

#endif