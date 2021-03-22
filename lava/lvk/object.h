#ifndef LVK_OBJECT_H
#define LVK_OBJECT_H

#include <vulkan/vulkan_core.h>

namespace lvk
{
    template <class T>
    class object
    {
    public:
        T vk() const { return vk_object; }
    protected:
        T vk_object = VK_NULL_HANDLE;
    };
}

#endif