#include "renderer.h"
#include "common.h"
#include "app.h"
#include "lvk.h"

lava_renderer::lava_renderer(lava_app * app)
{
    auto app_info = lvk::make_application_info(app->title, VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_2);

    // Query extensions needed by SDL
    uint32_t sdl_required_extension_count;
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, nullptr);
    std::vector<const char *> requested_extensions(sdl_required_extension_count);
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, requested_extensions.data());

    std::vector<const char *> requested_validation_layers;
#if USE_VALIDATION
    requested_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    requested_validation_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    auto instance_create_info = lvk::make_instance_create_info(&app_info, requested_extensions, requested_validation_layers);

    if (vkCreateInstance(&instance_create_info, nullptr, &vk_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }

#if USE_VALIDATION
    debug_messenger = lvk::make_default_debug_messenger(vk_instance);
#endif
}

lava_renderer::~lava_renderer()
{
#if USE_VALIDATION
    lvk::destroy_debug_messenger(vk_instance, debug_messenger);
#endif
    vkDestroyInstance(vk_instance, nullptr);
}