#include "device_selector.h"

namespace lvk
{
    physical_device::physical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
        : vk_physical_device(physical_device), surface(surface)
    {
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        api_version = properties.apiVersion;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
        for (uint32_t i = 0; i < memory_properties.memoryHeapCount; i++)
        {
            if (memory_properties.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                local_memory_size += memory_properties.memoryHeaps[i].size;
        }

        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
        queue_families.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_families.data());

        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
        extensions.resize(count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data());

        vkGetPhysicalDeviceFeatures(physical_device, &features);

        if (surface != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr);
            surface_formats.resize(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, surface_formats.data());

            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, nullptr);
            present_modes.resize(count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, present_modes.data());
        }
    }
    int32_t physical_device::find_queue_family(VkQueueFlags flags) const
    {
        for (size_t i = 0; i < queue_families.size(); i++)
            if (queue_families[i].queueFlags & flags)
                return i;
        return -1;
    }
    int32_t physical_device::find_exclusive_queue_family(VkQueueFlags flags) const
    {
        for (size_t i = 0; i < queue_families.size(); i++)
            if ((queue_families[i].queueFlags & flags) == flags)
                return i;
        return -1;
    }
    int32_t physical_device::find_present_supported_queue_family() const
    {
        for (uint32_t i = 0; i < queue_families.size(); i++)
            if (queue_family_supports_present(i))
                return i;
        return -1;
    }
    VkBool32 physical_device::queue_family_supports_present(uint32_t index) const
    {
        VkBool32 support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, index, surface, &support);
        return support;
    }
    bool physical_device::supports_features(VkPhysicalDeviceFeatures requested_features) const
    {
        if ((requested_features.robustBufferAccess && !features.robustBufferAccess) ||
            (requested_features.fullDrawIndexUint32 && !features.fullDrawIndexUint32) ||
            (requested_features.imageCubeArray && !features.imageCubeArray) ||
            (requested_features.independentBlend && !features.independentBlend) ||
            (requested_features.geometryShader && !features.geometryShader) ||
            (requested_features.tessellationShader && !features.tessellationShader) ||
            (requested_features.sampleRateShading && !features.sampleRateShading) ||
            (requested_features.dualSrcBlend && !features.dualSrcBlend) ||
            (requested_features.logicOp && !features.dualSrcBlend) ||
            (requested_features.multiDrawIndirect && !features.multiDrawIndirect) ||
            (requested_features.drawIndirectFirstInstance && !features.drawIndirectFirstInstance) ||
            (requested_features.depthClamp && !features.depthClamp) ||
            (requested_features.depthBiasClamp && !features.depthBiasClamp) ||
            (requested_features.fillModeNonSolid && !features.fillModeNonSolid) ||
            (requested_features.depthBounds && !features.depthBounds) ||
            (requested_features.wideLines && !features.wideLines) ||
            (requested_features.largePoints && !features.largePoints) ||
            (requested_features.alphaToOne && !features.alphaToOne) ||
            (requested_features.multiViewport && !features.multiViewport) ||
            (requested_features.samplerAnisotropy && !features.samplerAnisotropy) ||
            (requested_features.textureCompressionETC2 && !features.textureCompressionETC2) ||
            (requested_features.textureCompressionASTC_LDR && !features.textureCompressionASTC_LDR) ||
            (requested_features.textureCompressionBC && !features.textureCompressionBC) ||
            (requested_features.occlusionQueryPrecise && !features.occlusionQueryPrecise) ||
            (requested_features.pipelineStatisticsQuery && !features.pipelineStatisticsQuery) ||
            (requested_features.vertexPipelineStoresAndAtomics && !features.vertexPipelineStoresAndAtomics) ||
            (requested_features.fragmentStoresAndAtomics && !features.fragmentStoresAndAtomics) ||
            (requested_features.shaderTessellationAndGeometryPointSize && !features.shaderTessellationAndGeometryPointSize) ||
            (requested_features.shaderImageGatherExtended && !features.shaderImageGatherExtended) ||
            (requested_features.shaderStorageImageExtendedFormats && !features.shaderStorageImageExtendedFormats) ||
            (requested_features.shaderStorageImageMultisample && !features.shaderStorageImageMultisample) ||
            (requested_features.shaderStorageImageReadWithoutFormat && !features.shaderStorageImageReadWithoutFormat) ||
            (requested_features.shaderStorageImageWriteWithoutFormat && !features.shaderStorageImageWriteWithoutFormat) ||
            (requested_features.shaderUniformBufferArrayDynamicIndexing && !features.shaderUniformBufferArrayDynamicIndexing) ||
            (requested_features.shaderSampledImageArrayDynamicIndexing && !features.shaderSampledImageArrayDynamicIndexing) ||
            (requested_features.shaderStorageBufferArrayDynamicIndexing && !features.shaderStorageBufferArrayDynamicIndexing) ||
            (requested_features.shaderStorageImageArrayDynamicIndexing && !features.shaderStorageImageArrayDynamicIndexing) ||
            (requested_features.shaderClipDistance && !features.shaderClipDistance) ||
            (requested_features.shaderCullDistance && !features.shaderCullDistance) ||
            (requested_features.shaderFloat64 && !features.shaderFloat64) ||
            (requested_features.shaderInt64 && !features.shaderInt64) ||
            (requested_features.shaderInt16 && !features.shaderInt16) ||
            (requested_features.shaderResourceResidency && !features.shaderResourceResidency) ||
            (requested_features.shaderResourceMinLod && !features.shaderResourceMinLod) ||
            (requested_features.sparseBinding && !features.sparseBinding) ||
            (requested_features.sparseResidencyBuffer && !features.sparseResidencyBuffer) ||
            (requested_features.sparseResidencyImage2D && !features.sparseResidencyImage2D) ||
            (requested_features.sparseResidencyImage3D && !features.sparseResidencyImage3D) ||
            (requested_features.sparseResidency2Samples && !features.sparseResidency2Samples) ||
            (requested_features.sparseResidency4Samples && !features.sparseResidency4Samples) ||
            (requested_features.sparseResidency8Samples && !features.sparseResidency8Samples) ||
            (requested_features.sparseResidency16Samples && !features.sparseResidency16Samples) ||
            (requested_features.sparseResidencyAliased && !features.sparseResidencyAliased) ||
            (requested_features.variableMultisampleRate && !features.variableMultisampleRate) ||
            (requested_features.inheritedQueries && !features.inheritedQueries))
            return false;
        return true;
    }

    device_selector::device_selector(VkInstance instance, VkSurfaceKHR surface)
        : vk_instance(instance), surface(surface)
    {
        uint32_t count;
        vkEnumeratePhysicalDevices(vk_instance, &count, nullptr);
        if (count == 0)
            throw std::runtime_error("No devices with vulkan support detected");
        physical_devices.resize(count);
        vkEnumeratePhysicalDevices(vk_instance, &count, physical_devices.data());
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
            candidates.emplace_back(vk_physical_device, surface);

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
            if (candidate.find_queue_family(VK_QUEUE_GRAPHICS_BIT) == -1)
                return false;
            break;
        case queue_preference::dedicated:
            if (candidate.find_exclusive_queue_family(VK_QUEUE_GRAPHICS_BIT) == -1)
                return false;
            break;
        }

        switch (criteria.compute_queue_preference)
        {
        case queue_preference::supported:
            if (candidate.find_queue_family(VK_QUEUE_COMPUTE_BIT) == -1)
                return false;
            break;
        case queue_preference::dedicated:
            if (candidate.find_exclusive_queue_family(VK_QUEUE_COMPUTE_BIT) == -1)
                return false;
            break;
        }

        switch (criteria.transfer_queue_preference)
        {
        case queue_preference::supported:
            if (candidate.find_queue_family(VK_QUEUE_TRANSFER_BIT) == -1)
                return false;
            break;
        case queue_preference::dedicated:
            if (candidate.find_exclusive_queue_family(VK_QUEUE_TRANSFER_BIT) == -1)
                return false;
            break;
        }

        if (criteria.present_support && candidate.find_present_supported_queue_family() == -1)
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