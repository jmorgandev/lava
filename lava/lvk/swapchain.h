#ifndef LVK_SWAPCHAIN_H
#define LVK_SWAPCHAIN_H

#include "image_view.h"
#include <vulkan/vulkan.h>
#include <vector>

#include "object.h"

namespace lvk
{
    class device;
    class physical_device;

    class swapchain : public object<VkSwapchainKHR>
    {
    public:
        swapchain() = default;
        swapchain(VkSwapchainCreateInfoKHR create_info, VkDevice device);
        void destroy();
        ~swapchain(){}

        uint32_t size() const { return (uint32_t)images.size(); }
        VkFormat image_format() const { return info.imageFormat; }
        VkExtent2D image_extent() const { return info.imageExtent; }

        const std::vector<image_view> & get_image_views() { return image_views; }
    private:
        VkDevice vk_device;
        VkSwapchainCreateInfoKHR info;
        std::vector<VkImage> images;
        std::vector<image_view> image_views;
    };

    class swapchain_builder
    {
    public:
        swapchain_builder(const device & device, const physical_device & physical_device);

        swapchain_builder & size(uint32_t width, uint32_t height);
        swapchain_builder & image_usage(VkImageUsageFlags flags);
        swapchain_builder & composite_alpha(VkCompositeAlphaFlagBitsKHR flags);
        swapchain_builder & clipped(VkBool32 clipped);
        swapchain_builder & graphics_family_index(uint32_t index);
        swapchain_builder & present_family_index(uint32_t index);

        swapchain build();
    private:
        VkSwapchainCreateInfoKHR swapchain_info;
        uint32_t graphics_family;
        uint32_t present_family;
        const device & dev;
        const physical_device & phys_dev;
    };
}

#endif