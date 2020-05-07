#include "renderer.h"
#include "common.h"
#include "app.h"
#include "lvk.h"

#ifdef _DEBUG
static const std::vector<const char *> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};
#endif

lava_renderer::lava_renderer(lava_app * app)
{
    auto app_info = lvk::make_application_info(app->title, VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_2);

    // Query extensions needed by SDL
    uint32_t sdl_required_extension_count;
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, nullptr);
    std::vector<const char *> requested_extensions(sdl_required_extension_count);
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, requested_extensions.data());

    // Filter available extensions with the ones we requested
    auto available_extensions = vk::enumerateInstanceExtensionProperties();
    std::vector<const char *> supported_extension_names;
    std::copy_if(requested_extensions.begin(), requested_extensions.end(), std::back_inserter(supported_extension_names), [&available_extensions](const char * name) 
    {
        for (const auto & ext : available_extensions)
            if (strcmp(ext.extensionName, name) == 0) return true;
        return false;
    });

    auto instance_create_info = lvk::make_instance_create_info(&app_info, requested_extensions, {});
    setup_validation_layers(&instance_create_info);

    try
    {
        vk_instance = vk::createInstance(instance_create_info);
    }
    catch (std::exception e)
    {
        printf("Couldn't create vulkan instance: %s", e.what());
    }
}

lava_renderer::~lava_renderer()
{

}

void lava_renderer::setup_validation_layers(vk::InstanceCreateInfo * create_info)
{
#ifdef _DEBUG
    auto available_layers = vk::enumerateInstanceLayerProperties();

    for (const auto & requested_layer_name : validation_layers)
    {
        bool layer_found = false;
        for (const auto & layer_property : available_layers)
        {
            if (strcmp(requested_layer_name, layer_property.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
            throw "Some requested validation layers are not available!";
    }

    create_info->enabledLayerCount = (uint32_t)validation_layers.size();
    create_info->ppEnabledLayerNames = validation_layers.data();
#else
    create_info->enabledLayerCount = 0;
#endif
}