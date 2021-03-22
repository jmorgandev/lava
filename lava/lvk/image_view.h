#ifndef LVK_IMAGE_VIEW_H
#define LVK_IMAGE_VIEW_H

#include <vulkan/vulkan.h>
#include "object.h"

namespace lvk
{
    class device;

    class image_view : public object<VkImageView>
    {
    public:
        image_view() = default;
        image_view(VkImageViewCreateInfo create_info, VkDevice device);
    
        void destroy();
    private:
        VkDevice vk_device = VK_NULL_HANDLE;
        VkImageViewCreateInfo info;
    };
    class image_view_builder
    {
    public:
        image_view_builder(VkDevice device, VkImage image, VkFormat format);

        image_view_builder & view_type(VkImageViewType type);
        image_view_builder & aspect_flags(VkImageAspectFlags flags);
        image_view_builder & mip_levels(uint32_t levels);
        image_view_builder & base_mip_level(uint32_t level);
        image_view_builder & base_array_layer(uint32_t layer);
        image_view_builder & layer_count(uint32_t count);

        image_view build();
    private:
        VkDevice vk_device;
        VkImageViewCreateInfo create_info;
    };
}

#endif