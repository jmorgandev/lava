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
    if (vkCreateInstance(&instance_create_info, nullptr, &vk_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }

#if USE_VALIDATION
    debug_messenger = lvk::make_debug_messenger(vk_instance, &debug_create_info);
#endif
}

lava_renderer::~lava_renderer()
{
#if USE_VALIDATION
    lvk::destroy_debug_messenger(vk_instance, debug_messenger);
#endif
    vkDestroyInstance(vk_instance, nullptr);
}