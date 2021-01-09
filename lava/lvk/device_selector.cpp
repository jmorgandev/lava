#include "device_selector.h"

namespace lvk
{
    device_selector::device_selector(VkInstance instance)
        : vk_instance(instance), vk_surface(VK_NULL_HANDLE)
    {
        uint32_t count;
        vkEnumeratePhysicalDevices(vk_instance, &count, nullptr);
        if (count == 0)
            throw std::runtime_error("No devices with vulkan support detected");
        physical_devices.resize(count);
        vkEnumeratePhysicalDevices(vk_instance, &count, physical_devices.data());
    }
    device_selector & device_selector::render_surface(VkSurfaceKHR surface)
    {
        vk_surface = surface;
        return *this;
    }
    device_selector & device_selector::min_api_version(uint32_t api_version)
    {
        criteria.vulkan_api_version = api_version;
        return *this;
    }
    device_selector & device_selector::min_api_version(uint32_t major, uint32_t minor, uint32_t patch)
    {
        return min_api_version(VK_MAKE_VERSION(major, minor, patch));
    }
    device_selector & device_selector::with_features(VkPhysicalDeviceFeatures features)
    {
        criteria.features = features;
        return *this;
    }

    device_selector & device_selector::preset_graphics(VkDeviceSize minimum_memory)
    {
        criteria.device_preference = device_preference::discrete_gpu;
        criteria.graphics_queue_preference = queue_preference::supported;
        criteria.compute_queue_preference = queue_preference::any;
        criteria.transfer_queue_preference = queue_preference::supported;
        criteria.present_support = true;
        criteria.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        criteria.available_memory = minimum_memory;
        criteria.vulkan_api_version = VK_API_VERSION_1_0;
        return *this;
    }

    physical_device device_selector::select()
    {
        std::vector<physical_device> candidates;
        for (auto vk_physical_device : physical_devices)
            candidates.emplace_back(vk_physical_device, vk_surface);

        for (auto & candidate : candidates)
        {
            if (device_passes_criteria(candidate))
                return candidate;
        }
        throw std::runtime_error("No physical device supports selector criteria");
    }

    bool device_selector::device_passes_criteria(const physical_device & candidate)
    {
        if (criteria.vulkan_api_version > candidate.properties.apiVersion)
            return false;

        switch (criteria.graphics_queue_preference)
        {
        case queue_preference::supported:
            if (!candidate.has_compatible_queue_family(VK_QUEUE_GRAPHICS_BIT))
                return false;
            break;
        case queue_preference::dedicated:
            if (!candidate.has_exclusive_queue_family(VK_QUEUE_GRAPHICS_BIT))
                return false;
            break;
        }

        switch (criteria.compute_queue_preference)
        {
        case queue_preference::supported:
            if (!candidate.has_compatible_queue_family(VK_QUEUE_COMPUTE_BIT))
                return false;
            break;
        case queue_preference::dedicated:
            if (!candidate.has_exclusive_queue_family(VK_QUEUE_COMPUTE_BIT))
                return false;
            break;
        }

        switch (criteria.transfer_queue_preference)
        {
        case queue_preference::supported:
            if (!candidate.has_compatible_queue_family(VK_QUEUE_TRANSFER_BIT))
                return false;
            break;
        case queue_preference::dedicated:
            if (!candidate.has_exclusive_queue_family(VK_QUEUE_TRANSFER_BIT))
                return false;
            break;
        }

        if (criteria.present_support && !candidate.has_present_supported_queue_family())
            return false;

        for (auto extension : criteria.extensions)
        {
            bool supported = false;
            for (auto available_extension : candidate.extensions)
                supported |= strcmp(extension, available_extension.extensionName) == 0;
            if (!supported)
                return false;
        }

        if (criteria.present_support &&
            (candidate.surface_formats.empty() || candidate.present_modes.empty()))
            return false;

        if (criteria.device_preference != (device_preference)candidate.properties.deviceType)
            return false;

        if (!candidate.supports_features(criteria.features))
            return false;

        if (candidate.local_memory_size < criteria.available_memory)
            return false;

        return true;
    }
}