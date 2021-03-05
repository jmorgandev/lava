#ifndef LVK_QUEUE_H
#define LVK_QUEUE_H

#include <vulkan/vulkan.h>

namespace lvk
{
    struct queue
    {
        VkQueue vk_queue = VK_NULL_HANDLE;
        uint32_t family_index = 0;
    };
}

#endif