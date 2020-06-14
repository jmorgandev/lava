#include "renderer.h"

#include <set>
#include <SDL/SDL_video.h>

#include "app.h"
#include "lvk.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT * callback_data,
                                                     void * user_data)
{
    printf("%s\n", callback_data->pMessage);
    return VK_FALSE;
}

lava_renderer::lava_renderer(lava_app * app)
{
    // Register application information
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app->title;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    // Query extensions needed by SDL
    uint32_t sdl_required_extension_count;
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, nullptr);
    std::vector<const char *> requested_extensions(sdl_required_extension_count);
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, requested_extensions.data());
    std::vector<const char *> requested_layers;

#if USE_VALIDATION
    requested_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    requested_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    auto extensions = lvk::filter_supported_extensions(requested_extensions);
    auto layers = lvk::filter_supported_layers(requested_layers);
    
    // Create Vulkan instance
    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = extensions.size();
    instance_create_info.ppEnabledExtensionNames = extensions.data();
    instance_create_info.enabledLayerCount = layers.size();
    instance_create_info.ppEnabledLayerNames = layers.data();
    instance_create_info.pNext = nullptr;

#if USE_VALIDATION
    auto debug_create_info = lvk::make_default_debug_messenger_create_info();
    debug_create_info.pfnUserCallback = debug_callback;
    instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
#endif
    if (vkCreateInstance(&instance_create_info, nullptr, &vulkan_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }
    
#if USE_VALIDATION
    debug_messenger = lvk::make_debug_messenger(vulkan_instance, &debug_create_info);
#endif

    if (!SDL_Vulkan_CreateSurface(app->sdl_window, vulkan_instance, &window_surface))
        throw std::runtime_error("Failed to create window surface from SDL!");

    // Find and select vulkan compatible device for use
    uint32_t device_count;
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);
    if (device_count == 0)
        throw std::runtime_error("Couldn't find any GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, physical_devices.data());

    physical_device = VK_NULL_HANDLE;
    lvk::QueueFamilyInfo queue_family_info = {};
    std::vector<const char *> supported_device_extensions = {};
    // Select a GPU which has the following properties and features...
    for (const auto & device : physical_devices)
    {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);

        queue_family_info = lvk::get_queue_family_info(device, window_surface);
        if (queue_family_info.graphics_family != LVK_NULL_QUEUE_FAMILY)
        {
            supported_device_extensions = lvk::filter_supported_device_extensions(device, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
            if (!supported_device_extensions.empty())
            {
                lvk::DeviceSurfaceDetails surface_details = lvk::query_surface_details(device, window_surface);
                if (!surface_details.formats.empty() && !surface_details.present_modes.empty())
                {
                    physical_device = device;
                    break;
                }
            }
        }
    }

    if (physical_device == VK_NULL_HANDLE)
        throw std::runtime_error("Couldn't find a GPU with appropriate features!");

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<int> unique_families = { queue_family_info.graphics_family, queue_family_info.present_family };
    float priority = 1.0f;

    for (int family : unique_families)
    {
        VkDeviceQueueCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = family;
        create_info.queueCount = 1;
        create_info.pQueuePriorities = &priority;
        queue_create_infos.push_back(create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = (uint32_t)queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = (uint32_t)supported_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = supported_device_extensions.data();

#if DEVICE_VALIDATION_LAYER_COMPATIBILITY
    device_create_info.enabledLayerCount = (uint32_t)layers.size();
    device_create_info.ppEnabledLayerNames = layers.data();
#else
    // Device specific validation layers are deprecated as of Vulkan 1.2
    device_create_info.enabledLayerCount = 0; 
#endif

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    vkGetDeviceQueue(device, queue_family_info.graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, queue_family_info.present_family, 0, &present_queue);

    lvk::DeviceSurfaceDetails swapchain_support = lvk::query_surface_details(physical_device, window_surface);
    VkSurfaceFormatKHR swapchain_format = lvk::choose_swapchain_surface_format(swapchain_support.formats);
    VkPresentModeKHR swapchain_present_mode = lvk::choose_swapchain_present_mode(swapchain_support.present_modes);
    VkExtent2D swapchain_extent = lvk::choose_swapchain_extent(swapchain_support.capabilities, app->window_width, app->window_height);

    uint32_t swapchain_length = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        swapchain_length > swapchain_support.capabilities.maxImageCount)
    {
        swapchain_length = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = window_surface;
    swapchain_create_info.minImageCount = swapchain_length;
    swapchain_create_info.imageFormat = swapchain_format.format;
    swapchain_create_info.imageColorSpace = swapchain_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    if (queue_family_info.graphics_family != queue_family_info.present_family)
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        uint32_t indices[] = { queue_family_info.graphics_family, queue_family_info.present_family };
        swapchain_create_info.pQueueFamilyIndices = indices;
    }
    else
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    swapchain_create_info.preTransform = swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }
}

lava_renderer::~lava_renderer()
{
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(vulkan_instance, window_surface, nullptr);
    vkDestroyDevice(device, nullptr);
#if USE_VALIDATION
    lvk::destroy_debug_messenger(vulkan_instance, debug_messenger);
#endif
    vkDestroyInstance(vulkan_instance, nullptr);
}