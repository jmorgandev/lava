#ifndef LAVA_RENDERER_H
#define LAVA_RENDERER_H

#include <vulkan/vulkan.h>

class lava_app;

class lava_renderer
{
public:
    lava_renderer(lava_app * app);
    ~lava_renderer();

    void setup_validation_layers(VkInstanceCreateInfo * create_info);
    void setup_debug_messenger();

    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
};

#endif