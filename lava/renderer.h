#ifndef LAVA_RENDERER_H
#define LAVA_RENDERER_H

#include <vulkan/vulkan.hpp>

class lava_app;

class lava_renderer
{
public:
    lava_renderer(lava_app * app);
    ~lava_renderer();

    void setup_validation_layers(vk::InstanceCreateInfo * create_info);

    vk::Instance vk_instance;
};

#endif