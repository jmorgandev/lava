#include "device_selector.h"
#include "physical_device.h"

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
    device_selector & device_selector::prefer_device_type(device_preference type_preference)
    {
        criteria.type_preference = type_preference;
        return *this;
    }
    device_selector & device_selector::require_compute_queue()
    {
        criteria.compute_queue_preference = queue_preference::available;
        return *this;
    }
    device_selector & device_selector::require_separate_compute_queue()
    {
        criteria.compute_queue_preference = queue_preference::separate_from_graphics;
        return *this;
    }
    device_selector & device_selector::require_exclusive_compute_queue()
    {
        criteria.compute_queue_preference = queue_preference::exclusive;
        return *this;
    }
    device_selector & device_selector::require_transfer_queue()
    {
        criteria.transfer_queue_preference = queue_preference::available;
        return *this;
    }
    device_selector & device_selector::require_separate_transfer_queue()
    {
        criteria.transfer_queue_preference = queue_preference::separate_from_graphics;
        return *this;
    }
    device_selector & device_selector::require_exclusive_transfer_queue()
    {
        criteria.transfer_queue_preference = queue_preference::exclusive;
        return *this;
    }
    device_selector & device_selector::require_present_support()
    {
        criteria.present_support = true;
        return *this;
    }
    device_selector & device_selector::with_extension(const char * name)
    {
        criteria.extensions.emplace_back(name);
        return *this;
    }
    device_selector & device_selector::with_extensions(const std::vector<const char *> & names)
    {
        criteria.extensions.insert(criteria.extensions.end(), names.begin(), names.end());
        return *this;
    }
    device_selector & device_selector::minimum_memory(VkDeviceSize size)
    {
        criteria.available_memory = size;
        return *this;
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

        switch (criteria.compute_queue_preference)
        {
        case queue_preference::available:
            if (!candidate.has_compatible_queue_family(VK_QUEUE_COMPUTE_BIT))
                return false;
            break;
        case queue_preference::separate_from_graphics:
            if (!candidate.has_mutually_exclusive_queue_family(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT))
                return false;
            break;
        case queue_preference::exclusive:
            if (!candidate.has_exclusive_queue_family(VK_QUEUE_COMPUTE_BIT))
                return false;
            break;
        }

        switch (criteria.transfer_queue_preference)
        {
        case queue_preference::available:
            if (!candidate.has_compatible_queue_family(VK_QUEUE_TRANSFER_BIT))
                return false;
            break;
        case queue_preference::separate_from_graphics:
            if (!candidate.has_mutually_exclusive_queue_family(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT))
                return false;
            break;
        case queue_preference::exclusive:
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

        auto surface_details = candidate.query_surface_details();

        if (criteria.present_support && 
            (surface_details.formats.empty() || surface_details.present_modes.empty()))
            return false;

        if (criteria.type_preference != (device_preference)candidate.properties.deviceType)
            return false;

        if (!candidate.supports_features(criteria.features))
            return false;

        if (candidate.local_memory_size < criteria.available_memory)
            return false;

        return true;
    }
}