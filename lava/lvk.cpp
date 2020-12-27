#include "lvk.h"
#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>
#include <cstdint>
#include <bitset>

std::vector<const char *> lvk::filter_supported_extensions(std::vector<const char *> requested_extensions)
{
    uint32_t supported_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, nullptr);
    std::vector<VkExtensionProperties> supported_extensions(supported_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, supported_extensions.data());
    std::vector<const char *> filtered_extensions;
    std::copy_if(requested_extensions.begin(), requested_extensions.end(), std::back_inserter(filtered_extensions),
        [&supported_extensions](const char * name) -> bool
    {
        for (const auto & ext : supported_extensions)
            if (strcmp(ext.extensionName, name) == 0) return true;
        return false;
    });
    return filtered_extensions;
}

std::vector<const char *> lvk::filter_supported_layers(std::vector<const char *> requested_layers)
{
    uint32_t supported_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&supported_layer_count, nullptr);
    std::vector<VkLayerProperties> supported_layers(supported_layer_count);
    vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers.data());
    std::vector<const char *> filtered_layers;
    std::copy_if(requested_layers.begin(), requested_layers.end(), std::back_inserter(filtered_layers),
        [&supported_layers](const char * name) -> bool
    {
        for (const auto & layer : supported_layers)
            if (strcmp(layer.layerName, name) == 0) return true;
        return false;
    });
    return filtered_layers;
}

std::vector<const char *> lvk::filter_supported_device_extensions(VkPhysicalDevice device, std::vector<const char *> requested_extensions)
{
    uint32_t supported_extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &supported_extension_count, nullptr);
    std::vector<VkExtensionProperties> supported_extensions(supported_extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &supported_extension_count, supported_extensions.data());
    std::vector<const char *> filtered_extensions;
    std::copy_if(requested_extensions.begin(), requested_extensions.end(), std::back_inserter(filtered_extensions),
        [&supported_extensions](const char * name) -> bool
    {
        for (const auto & ext : supported_extensions)
            if (strcmp(ext.extensionName, name) == 0) return true;
        return false;
    });
    return filtered_extensions;
}

void lvk::destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debug_messenger, allocator);
    }
}

lvk::QueueFamilyInfo lvk::get_queue_family_info(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyInfo info = { LVK_NULL_QUEUE_FAMILY, LVK_NULL_QUEUE_FAMILY };
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());
    int current_index = 0;
    for (const auto & family : properties)
    {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            info.graphics_family = current_index;

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, current_index, surface, &present_support);
        if (present_support)
            info.present_family = current_index;

        if (info.graphics_family != LVK_NULL_QUEUE_FAMILY && info.present_family != LVK_NULL_QUEUE_FAMILY)
            break;

        current_index++;
    }
    return info;
}

lvk::DeviceSurfaceDetails lvk::query_surface_details(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    DeviceSurfaceDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());

    return details;
}

VkSurfaceFormatKHR lvk::choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> & available_formats)
{
    for (const auto & surface_format : available_formats)
    {
        if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surface_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR lvk::choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> & available_modes)
{
    for (const auto & mode : available_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D lvk::choose_swapchain_extent(const VkSurfaceCapabilitiesKHR & capabilities, uint32_t window_width, uint32_t window_height)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D extent = { window_width, window_height };
        extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
        return extent;
    }
}

VkShaderModule lvk::create_shader_module(VkDevice device, const std::vector<char> & source)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = source.size();
    create_info.pCode = (uint32_t *)(source.data());

    VkShaderModule module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &module) != VK_SUCCESS)
        throw std::runtime_error("Error creating shader module");

    return module;
}

uint32_t lvk::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    throw std::runtime_error("failed to find suitable memory type!");
}

// START OF REFACTOR

VKAPI_ATTR VkBool32 VKAPI_CALL lvk::debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT * callback_data, void * user_data)
{
    printf("%s\n", callback_data->pMessage);
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        throw std::runtime_error(callback_data->pMessage);
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT lvk::default_debug_messenger_create_info()
{
    VkDebugUtilsMessengerCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pUserData = nullptr;
    info.pfnUserCallback = lvk::debug_messenger_callback;
    return info;
}

VkInstance lvk::create_instance(uint32_t api_version, std::vector<const char *> extensions, std::vector<const char *> layers, const void * pNext, const char * app_name)
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = api_version;

    VkInstanceCreateInfo instance_info = {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledExtensionCount = (uint32_t)extensions.size();
    instance_info.ppEnabledExtensionNames = extensions.data();
    instance_info.enabledLayerCount = (uint32_t)layers.size();
    instance_info.ppEnabledLayerNames = layers.data();
    instance_info.pNext = pNext;
    
    VkInstance instance;
    vkCreateInstance(&instance_info, nullptr, &instance);
    return instance;
}

VkDebugUtilsMessengerEXT lvk::create_debug_messenger(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT * create_info)
{
    auto info = (create_info) ? *create_info : lvk::default_debug_messenger_create_info();
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        if (func(instance, &info, nullptr, &debug_messenger) != VK_SUCCESS)
            throw std::runtime_error("Couldn't create default debug messenger!");
    }
    else throw std::runtime_error("Couldn't locate proc addr of \"vkCreateDebugUtilsMessengerEXT\"!");
    return debug_messenger;
}

lvk::PhysicalDeviceDetails lvk::get_physical_device_details(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    lvk::PhysicalDeviceDetails details = {};
    uint32_t length = 0;

    vkGetPhysicalDeviceProperties(device, &details.device_properties);
    
    vkGetPhysicalDeviceMemoryProperties(device, &details.memory_properties);
    for (uint32_t i = 0; i < details.memory_properties.memoryTypeCount; i++)
    {
        VkMemoryType type = details.memory_properties.memoryTypes[i];
        if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            details.local_memory_size = details.memory_properties.memoryHeaps[type.heapIndex].size;
        }
   }
    for (uint32_t i = 0; i < details.memory_properties.memoryHeapCount; i++)
        details.total_memory_size += details.memory_properties.memoryHeaps[i].size;

    auto queue_family_flag_search = [](auto & family, VkQueueFlags flag) { return family.queueFlags & flag };

    vkGetPhysicalDeviceQueueFamilyProperties(device, &length, nullptr);
    details.queue_families.resize(length);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &length, details.queue_families.data());

    vkEnumerateDeviceExtensionProperties(device, nullptr, &length, nullptr);
    details.extensions.resize(length);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &length, details.extensions.data());
    for (const auto & extension : details.extensions)
    {
        if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            details.supports_swapchain = true;
    }

    if (surface != VK_NULL_HANDLE)
    {
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &length, nullptr);
        details.surface_formats.resize(length);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &length, details.surface_formats.data());

        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &length, nullptr);
        details.present_modes.resize(length);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &length, details.present_modes.data());
    }

    // Provide extra information
    
    for (uint32_t i = 0; i < details.queue_families.size(); i++)
    {
        VkQueueFlags flags = details.queue_families[i].queueFlags;
        details.supports_graphics = details.supports_graphics || (flags & VK_QUEUE_GRAPHICS_BIT);
        details.supports_compute =  details.supports_compute || (flags & VK_QUEUE_COMPUTE_BIT);
        
        VkBool32 pSupported;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &pSupported);
        details.supports_present = details.supports_present || pSupported;
    }

    return details;
}