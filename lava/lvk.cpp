#include "lvk.h"
#include <algorithm>
#include <iterator>
#include "common.h"

namespace lvk
{
    VkApplicationInfo make_application_info(const char * app_title, uint32_t app_version, uint32_t api_version)
    {
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = app_title;
        app_info.applicationVersion = app_version;
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = api_version;
        return app_info;
    }

    std::vector<const char *> filter_supported_extensions(std::vector<const char *> requested_extensions)
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

    std::vector<const char *> filter_supported_layers(std::vector<const char *> requested_layers)
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

    VkDebugUtilsMessengerEXT make_debug_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * create_info, const VkAllocationCallbacks * allocator)
    {
        VkDebugUtilsMessengerEXT debug_messenger;
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            if (func(instance, create_info, allocator, &debug_messenger) != VK_SUCCESS)
                throw std::runtime_error("Couldn't create default debug messenger!");
        }
        else
        {
            throw std::runtime_error("Couldn't locate proc addr of \"vkCreateDebugUtilsMessengerEXT\"!");
        }
        return debug_messenger;
    }

    VkDebugUtilsMessengerCreateInfoEXT make_default_debug_messenger_create_info()
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pUserData = nullptr;
        return create_info;
    }

    void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debug_messenger, allocator);
        }
    }

    QueueFamilyInfo get_queue_family_info(VkPhysicalDevice device)
    {
        QueueFamilyInfo info;
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> properties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());
        int current_index = 0;
        for (const auto & family : properties)
        {
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT && info.graphics_queue_family == LVK_NULL_QUEUE_FAMILY)
                info.graphics_queue_family = current_index;

            current_index++;
        }
        return info;
    }
}