#ifndef LVK_H
#define LVK_H

#include <vulkan/vulkan.hpp>
#include <SDL/SDL_vulkan.h>
#include <vector>

namespace lvk
{
    vk::ApplicationInfo make_application_info(const char * app_title, uint32_t app_version, uint32_t api_version);
    vk::InstanceCreateInfo make_instance_create_info(vk::ApplicationInfo * app_info, const std::vector<const char *> & extensions, const std::vector<const char *> & validation_layers);
}

#endif