#include "device.h"

namespace lvk
{
    physical_device::physical_device() : vk_physical_device(VK_NULL_HANDLE), surface(VK_NULL_HANDLE) {}
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
    bool physical_device::has_compatible_queue_family(VkQueueFlags flags) const
    {
        for (auto & family : queue_families)
            if (family.queueFlags & flags)
                return true;
        return false;
    }
    bool physical_device::has_mutually_exclusive_queue_family(VkQueueFlags flags, VkQueueFlags exclude_flags) const
    {
        for (auto & family : queue_families)
            if ((family.queueFlags & flags) == flags &&
                (family.queueFlags & exclude_flags) != exclude_flags)
                return true;
        return false;
    }
    bool physical_device::has_exclusive_queue_family(VkQueueFlags flags) const
    {
        for (auto & family : queue_families)
            if ((family.queueFlags & flags) == flags)
                return true;
        return false;
    }
    bool physical_device::has_present_supported_queue_family() const
    {
        for (uint32_t i = 0; i < queue_families.size(); i++)
            if (queue_family_supports_present(i))
                return true;
        return false;
    }
    VkBool32 physical_device::queue_family_supports_present(uint32_t index) const
    {
        if (surface == VK_NULL_HANDLE)
            return false;
        VkBool32 support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, index, surface, &support);
        return support;
    }
    uint32_t physical_device::compatible_queue_family_index(VkQueueFlags flags) const
    {
        for (uint32_t i = 0; i < queue_families.size(); i++)
            if (queue_families[i].queueFlags & flags)
                return i;
        throw std::runtime_error("Physical device does not have compatible queue family");
    }
    uint32_t physical_device::mutually_exclusive_queue_family_index(VkQueueFlags flags, VkQueueFlags exclude_flags) const
    {
        for (uint32_t i = 0; i < queue_families.size(); i++)
            if ((queue_families[i].queueFlags & flags) == flags &&
                (queue_families[i].queueFlags & exclude_flags) != exclude_flags)
                return i;
        throw std::runtime_error("Physical device does not have mutually exclusive queue family");
    }
    uint32_t physical_device::exclusive_queue_family_index(VkQueueFlags flags) const
    {
        for (uint32_t i = 0; i < queue_families.size(); i++)
            if ((queue_families[i].queueFlags & flags) == flags)
                return i;
        throw std::runtime_error("Physical device does not have exclusive queue family");
    }
    uint32_t physical_device::present_queue_family_index() const
    {
        for (uint32_t i = 0; i < queue_families.size(); i++)
            if (queue_family_supports_present(i))
                return i;
        throw std::runtime_error("Physical device does not have present supported queue family");
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
    VkSampleCountFlagBits physical_device::max_usable_sample_count() const
    {
        VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
        constexpr VkSampleCountFlagBits count_priorities[] =
        {
            VK_SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_32_BIT, VK_SAMPLE_COUNT_16_BIT,
            VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT
        };
        for (auto item : count_priorities)
            if (counts & item) return item;
        return VK_SAMPLE_COUNT_1_BIT;
    }
}