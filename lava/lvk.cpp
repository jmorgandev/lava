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

    VkInstanceCreateInfo make_instance_create_info(VkApplicationInfo * app_info, const std::vector<const char *> & requested_extensions, const std::vector<const char *> & requested_validation_layers)
    {
        using std::begin;
        using std::end;
        VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        create_info.pApplicationInfo = app_info;
        if (!requested_extensions.empty())
        {
            uint32_t available_extension_count = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
            std::vector<VkExtensionProperties> available_extensions(available_extension_count);
            vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());
            std::vector<const char *> supported_extensions;
            std::copy_if(requested_extensions.begin(), requested_extensions.end(), std::back_inserter(supported_extensions),
                         [&available_extensions](const char * name) -> bool
            {
                for (const auto & ext : available_extensions)
                    if (strcmp(ext.extensionName, name) == 0) return true;
                return false;
            });
            create_info.enabledExtensionCount = (uint32_t)supported_extensions.size();
            create_info.ppEnabledExtensionNames = supported_extensions.data();
        }

        if (!requested_validation_layers.empty())
        {
            uint32_t available_layer_count = 0;
            vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
            std::vector<VkLayerProperties> available_layers(available_layer_count);
            vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data());
            for (const auto & requested_layer : requested_validation_layers)
            {
                bool found = false;
                for (const auto & available_layer : available_layers)
                {
                    if (strcmp(requested_layer, available_layer.layerName) == 0)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    throw std::runtime_error(std::string("Unsupported validation layer: ") + std::string(requested_layer));
            }
            create_info.enabledLayerCount = (uint32_t)requested_validation_layers.size();
            create_info.ppEnabledLayerNames = requested_validation_layers.data();
        }

        create_info.pNext = nullptr;
        return create_info;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                         VkDebugUtilsMessageTypeFlagsEXT type,
                                                         const VkDebugUtilsMessengerCallbackDataEXT * callback_data,
                                                         void * user_data)
    {
        printf("%s\n", callback_data->pMessage);
        return VK_FALSE;
    }

    VkDebugUtilsMessengerEXT make_default_debug_messenger(VkInstance instance, const VkAllocationCallbacks * allocator = nullptr)
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
        create_info.pUserData = nullptr;

        VkDebugUtilsMessengerEXT debug_messenger;
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            if (func(instance, &create_info, allocator, &debug_messenger) != VK_SUCCESS)
                throw std::runtime_error("Couldn't create default debug messenger!");
        }
        else
        {
            throw std::runtime_error("Couldn't locate proc addr of \"vkCreateDebugUtilsMessengerEXT\"!");
        }
        return debug_messenger;
    }

    void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debug_messenger, allocator)
        }
    }
}