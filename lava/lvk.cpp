#include "lvk.h"

namespace lvk
{
    vk::ApplicationInfo make_application_info(const char * app_title, uint32_t app_version, uint32_t api_version)
    {
        return vk::ApplicationInfo(app_title, app_version, "No Engine", VK_MAKE_VERSION(1, 0, 0), api_version);
    }

    vk::InstanceCreateInfo make_instance_create_info(vk::ApplicationInfo * app_info, const std::vector<const char *> & extensions, const std::vector<const char *> & validation_layers)
    {
        vk::InstanceCreateInfo create_info = {};
        create_info.pApplicationInfo = app_info;
        if (!extensions.empty())
        {
            create_info.enabledExtensionCount = (uint32_t)extensions.size();
            create_info.ppEnabledExtensionNames = extensions.data();
        }

        if (!validation_layers.empty())
        {
            create_info.enabledLayerCount = (uint32_t)validation_layers.size();
            create_info.ppEnabledLayerNames = validation_layers.data();
        }

        create_info.pNext = nullptr;
        return create_info;
    }
}