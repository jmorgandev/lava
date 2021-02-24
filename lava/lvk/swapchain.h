#ifndef LVK_SWAPCHAIN_H
#define LVK_SWAPCHAIN_H

#include <vulkan/vulkan.h>

namespace lvk
{
    class device;
    class physical_device;

    class swapchain
    {
    public:
        swapchain() = default;
        swapchain(VkSwapchainKHR swapchain, VkDevice device);
        void destroy();
        ~swapchain(){}

        VkSwapchainKHR vk() const { return vk_swapchain; }
    private:
        VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
        VkDevice vk_device = VK_NULL_HANDLE;
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