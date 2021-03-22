#include "image_view.h"
#include "device.h"

namespace lvk
{
    image_view_builder::image_view_builder(VkDevice device, VkImage image, VkFormat format)
        : vk_device(device)
    {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = image;
        create_info.format = format;
    }
    image_view_builder & image_view_builder::view_type(VkImageViewType type)
    {
        create_info.viewType = type;
        return *this;
    }
    image_view_builder & image_view_builder::aspect_flags(VkImageAspectFlags flags)
    {
        create_info.subresourceRange.aspectMask = flags;
        return *this;
    }
    image_view_builder & image_view_builder::mip_levels(uint32_t levels)
    {
        create_info.subresourceRange.levelCount = levels;
        return *this;
    }
    image_view_builder & image_view_builder::base_mip_level(uint32_t level)
    {
        create_info.subresourceRange.baseMipLevel = level;
        return *this;
    }
    image_view_builder & image_view_builder::base_array_layer(uint32_t layer)
    {
        create_info.subresourceRange.baseArrayLayer = layer;
        return *this;
    }
    image_view_builder & image_view_builder::layer_count(uint32_t count)
    {
        create_info.subresourceRange.layerCount = count;
        return *this;
    }
    image_view image_view_builder::build()
    {
        return image_view(create_info, vk_device);
    }

    image_view::image_view(VkImageViewCreateInfo create_info, VkDevice device)
        : vk_device(device), info(create_info)
    {
        VkResult result = vkCreateImageView(vk_device, &info, nullptr, &vk_object);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view");
    }
    void image_view::destroy()
    {
        vkDestroyImageView(vk_device, vk_object, nullptr);
        vk_object = VK_NULL_HANDLE;
    }
}