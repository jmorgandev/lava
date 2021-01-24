#include "device.h"
#include "physical_device.h"

namespace lvk
{
    device_builder::device_builder(const physical_device & physical_device)
        : phys_device(physical_device)
    {

    }
    device_builder & device_builder::queues(uint32_t family_index, uint32_t count, std::vector<float> priorities)
    {
        queue_infos_and_priorities.second.push_back(priorities);
        VkDeviceQueueCreateInfo info = {};
        auto & priors = queue_infos_and_priorities.second.back();

        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family_index;
        info.queueCount = count;
        info.pQueuePriorities = priors.data();
        queue_infos_and_priorities.first.push_back(info);
        return *this;
    }
    device_builder & device_builder::queues(uint32_t family_index, uint32_t count)
    {
        std::vector<float> priorities(count, 1.0f);
        return queues(family_index, count, priorities);
    }
    device_builder & device_builder::extension(const char * name)
    {
        enabled_extensions.push_back(name);
        return *this;
    }
    device_builder & device_builder::extensions(std::vector<const char *> names)
    {
        enabled_extensions.insert(enabled_extensions.end(), names.begin(), names.end());
        return *this;
    }
    device_builder & device_builder::features(VkPhysicalDeviceFeatures features)
    {
        enabled_features = features;
        return *this;
    }
    device device_builder::build()
    {
        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount = (uint32_t)queue_infos_and_priorities.first.size();
        info.pQueueCreateInfos = queue_infos_and_priorities.first.data();
        info.pEnabledFeatures = &enabled_features;
        info.enabledExtensionCount = (uint32_t)enabled_extensions.size();
        info.ppEnabledExtensionNames = enabled_extensions.data();
        info.enabledLayerCount = 0;

        VkDevice vk_device;
        VkResult result = vkCreateDevice(phys_device.vk(), &info, nullptr, &vk_device);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device");

        return device(vk_device, phys_device);
    }

    device::device() : vk_device(VK_NULL_HANDLE) {}
    device::device(VkDevice device, const physical_device & physical_device)
        : vk_device(device), phys_device(physical_device)
    {

    }
    device::device(device && rhs)
    {
        vk_device = rhs.vk_device;
        phys_device = rhs.phys_device;
        rhs.vk_device = VK_NULL_HANDLE;
        rhs.phys_device = physical_device();
    }
    device & device::operator=(device && rhs)
    {
        vk_device = rhs.vk_device;
        phys_device = rhs.phys_device;
        rhs.vk_device = VK_NULL_HANDLE;
        rhs.phys_device = physical_device();
        return *this;
    }
    void device::destroy()
    {
        vkDestroyDevice(vk_device, nullptr);
        vk_device = VK_NULL_HANDLE;
    }
}