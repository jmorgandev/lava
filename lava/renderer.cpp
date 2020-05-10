#include "renderer.h"
#include "common.h"
#include "app.h"
#include "lvk.h"

#if USE_VALIDATION
static const std::vector<const char *> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT * callback_data,
                                                     void * user_data)
{
    printf("%s\n", callback_data->pMessage);
    return VK_FALSE;
}

VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * create_info,
                                          const VkAllocationCallbacks * allocator, VkDebugUtilsMessengerEXT * debug_messenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, create_info, allocator, debug_messenger);
    }
    else return VK_ERROR_EXTENSION_NOT_PRESENT;
}
VkResult destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks * allocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debug_messenger, allocator);
    }
}
#endif

lava_renderer::lava_renderer(lava_app * app)
{
    auto app_info = lvk::make_application_info(app->title, VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_2);

    // Query extensions needed by SDL
    uint32_t sdl_required_extension_count;
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, nullptr);
    std::vector<const char *> requested_extensions(sdl_required_extension_count);
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, requested_extensions.data());

#if USE_VALIDATION
    requested_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    std::vector<const char *> requested_validation_layers = { "VK_LAYER_KHRONOS_validation" };

    auto instance_create_info = lvk::make_instance_create_info(&app_info, requested_extensions, requested_validation_layers);

    if (vkCreateInstance(&instance_create_info, nullptr, &vk_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }

    setup_debug_messenger();
}

lava_renderer::~lava_renderer()
{
#if _DEBUG
    destroy_debug_utils_messenger_ext(vk_instance, debug_messenger, nullptr);
#endif
    vkDestroyInstance(vk_instance, nullptr);
}

void lava_renderer::setup_debug_messenger()
{
#if _DEBUG
    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData = nullptr;

    create_debug_utils_messenger_ext(vk_instance, &create_info, nullptr, &debug_messenger);
#endif
}