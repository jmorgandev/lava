#include "renderer.h"
#include "common.h"
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

    // Find and select vulkan compatible device for use
    uint32_t device_count;
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);
    if (device_count == 0)
        throw std::runtime_error("Couldn't find any GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, physical_devices.data());

    physical_device = VK_NULL_HANDLE;
    // Select a GPU which has the following properties and features...
    for (const auto & device : physical_devices)
    {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);

        auto queue_family_info = lvk::get_queue_family_info(device);
        if (queue_family_info.graphics_queue_family != LVK_NULL_QUEUE_FAMILY)
        {
            physical_device = device;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE)
        throw std::runtime_error("Couldn't find a GPU with appropriate features!");

    lvk::QueueFamilyInfo queue_family_info = lvk::get_queue_family_info(physical_device);

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_info.graphics_queue_family;
    float priority = 1.0f;
    queue_create_info.pQueuePriorities = &priority;

    VkPhysicalDeviceFeatures device_features;

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = &device_features;

#if DEVICE_VALIDATION_LAYER_COMPATIBILITY
    device_create_info.enabledLayerCount = (uint32_t)layers.size();
    device_create_info.ppEnabledLayerNames = layers.data();
#else
    // Device specific validation layers are deprecated as of Vulkan 1.2
    device_create_info.enabledLayerCount = 0; 
#endif

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    vkGetDeviceQueue(device, queue_family_info.graphics_queue_family, 0, &graphics_queue);

    if (!SDL_Vulkan_CreateSurface(app->sdl_window, vulkan_instance, &window_surface))
        throw std::runtime_error("Failed to create window surface from SDL!");
}

lava_renderer::~lava_renderer()
{
    vkDestroySurfaceKHR(vulkan_instance, window_surface, nullptr);
    vkDestroyDevice(device, nullptr);
#if USE_VALIDATION
    lvk::destroy_debug_messenger(vulkan_instance, debug_messenger);
#endif
    vkDestroyInstance(vulkan_instance, nullptr);
}