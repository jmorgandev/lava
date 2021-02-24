#include "instance.h"
#include <iostream>
#include <algorithm>
#include <SDL/SDL_video.h>
#include <SDL/SDL_vulkan.h>
#include "device_selector.h"

VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT * callback_data,
    void * user_data)
{
    std::cout << callback_data->pMessage << std::endl;
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        throw std::runtime_error(callback_data->pMessage);
    return VK_FALSE;
}

namespace lvk
{
    instance_builder::instance_builder()
    {
        app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "";
        app_info.pEngineName = "";

        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        using_debug_messenger = false;
    }

    instance_builder & instance_builder::application_name(const char * name)
    {
        app_info.pApplicationName = name;
        return *this;
    }
    instance_builder & instance_builder::application_version(uint32_t major, uint32_t minor, uint32_t patch)
    {
        app_info.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
        return *this;
    }
    instance_builder & instance_builder::engine_name(const char * name)
    {
        app_info.pEngineName = name;
        return *this;
    }
    instance_builder & instance_builder::engine_version(uint32_t major, uint32_t minor, uint32_t patch)
    {
        app_info.engineVersion = VK_MAKE_VERSION(major, minor, patch);
        return *this;
    }
    instance_builder & instance_builder::api_version(uint32_t version)
    {
        app_info.apiVersion = version;
        return *this;
    }
    instance_builder & instance_builder::api_version(uint32_t major, uint32_t minor, uint32_t patch)
    {
        return api_version(VK_MAKE_VERSION(major, minor, patch));
    }
    instance_builder & instance_builder::extension(const char * name)
    {
        requested_extensions.push_back(name);
        return *this;
    }
    instance_builder & instance_builder::extensions(const std::vector<const char *> & names)
    {
        requested_extensions.insert(requested_extensions.end(), names.begin(), names.end());
        return *this;
    }
    instance_builder & instance_builder::layer(const char * name)
    {
        requested_layers.push_back(name);
        return *this;
    }
    instance_builder & instance_builder::layers(const std::vector<const char *> & names)
    {
        requested_layers.insert(requested_layers.end(), names.begin(), names.end());
        return *this;
    }
    instance_builder & instance_builder::default_debug_messenger()
    {
        using_debug_messenger = true;
        debug_info = {};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_info.pUserData = nullptr;
        debug_info.pfnUserCallback = default_debug_messenger_callback;
        return *this;
    }
    instance_builder & instance_builder::custom_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT info)
    {
        using_debug_messenger = true;
        debug_info = info;
        return *this;
    }

    instance instance_builder::build()
    {
        uint32_t count = 0;
        if (!requested_extensions.empty())
        {
            vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> available_extensions(count);
            vkEnumerateInstanceExtensionProperties(nullptr, &count, available_extensions.data());
            for (const auto & requested_ext : requested_extensions)
            {
                bool extension_supported = false;
                for (auto available_ext : available_extensions)
                    if (strcmp(requested_ext, available_ext.extensionName) == 0)
                        extension_supported = true;
                if (!extension_supported)
                    throw std::runtime_error(std::string("Vulkan instance does not support requested extension: ") + requested_ext);
            }
        }
        if (!requested_layers.empty())
        {
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> available_layers(count);
            vkEnumerateInstanceLayerProperties(&count, available_layers.data());
            for (const auto & requested_layer : requested_layers)
            {
                bool layer_supported = false;
                for (const auto & available_layer : available_layers)
                    if (strcmp(requested_layer, available_layer.layerName) == 0)
                        layer_supported = true;
                if (!layer_supported)
                    throw std::runtime_error(std::string("Vulkan instance does not supported requested layer: ") + requested_layer);
            }
        }

        create_info.enabledExtensionCount = (uint32_t)requested_extensions.size();
        create_info.ppEnabledExtensionNames = requested_extensions.data();
        create_info.enabledLayerCount = (uint32_t)requested_layers.size();
        create_info.ppEnabledLayerNames = requested_layers.data();
        if (using_debug_messenger)
        {
            // Temporary debug messenger to capture instance creation errors
            create_info.pNext = &debug_info;
        }

        VkInstance vk_instance;
        VkResult result = vkCreateInstance(&create_info, nullptr, &vk_instance);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create vulkan instance");

        VkDebugUtilsMessengerEXT vk_debug_messenger = VK_NULL_HANDLE;
        if (using_debug_messenger)
        {
            // Create the actual debug messenger to be used during runtime
            auto createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT");
            result = createDebugUtilsMessengerEXT(vk_instance, &debug_info, nullptr, &vk_debug_messenger);
            if (result != VK_SUCCESS)
                throw std::runtime_error("Failed to create debug messenger");
        }
        return instance(vk_instance, vk_debug_messenger);
    }

    instance::instance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
        : vk_instance(instance), vk_debug_messenger(debug_messenger)
    {
    }
    void instance::destroy()
    {
        if (vk_debug_messenger != VK_NULL_HANDLE)
        {
            auto destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT");
            destroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr);
        }

        vkDestroyInstance(vk_instance, nullptr);
    }
    VkSurfaceKHR instance::create_sdl_window_surface(SDL_Window * sdl_window)
    {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(sdl_window, vk_instance, &surface))
            throw std::runtime_error("Failed to create SDL window surface");
        return surface;
    }
    device_selector instance::select_physical_device()
    {
        return device_selector(vk_instance);
    }
}