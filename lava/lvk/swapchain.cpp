#include "swapchain.h"

#include "device.h"
#include "physical_device.h"

namespace lvk
{
    swapchain_builder::swapchain_builder(const device & device, const physical_device & phys_device)
        : dev(device), phys_dev(phys_device)
    {
        swapchain_info = {};
        swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    }
    swapchain_builder & swapchain_builder::size(uint32_t width, uint32_t height)
    {
        swapchain_info.imageExtent = phys_dev.choose_swapchain_extent(width, height);
        return *this;
    }
    swapchain_builder & swapchain_builder::image_usage(VkImageUsageFlags flags)
    {
        swapchain_info.imageUsage = flags;
        return *this;
    }
    swapchain_builder & swapchain_builder::composite_alpha(VkCompositeAlphaFlagBitsKHR flags)
    {
        swapchain_info.compositeAlpha = flags;
        return *this;
    }
    swapchain_builder & swapchain_builder::clipped(VkBool32 clipped)
    {
        swapchain_info.clipped = clipped;
        return *this;
    }
    swapchain_builder & swapchain_builder::graphics_family_index(uint32_t index)
    {
        graphics_family = index;
        return *this;
    }
    swapchain_builder & swapchain_builder::present_family_index(uint32_t index)
    {
        present_family = index;
        return *this;
    }
    swapchain swapchain_builder::build()
    {
        VkSurfaceFormatKHR swapchain_format = phys_dev.choose_swapchain_surface_format();
        VkPresentModeKHR swapchain_present_mode = phys_dev.choose_swapchain_present_mode();
        uint32_t swapchain_length = phys_dev.choose_swapchain_length();

        surface_details surface_details = phys_dev.query_surface_details();

        swapchain_info.surface = phys_dev.vk_surface;
        swapchain_info.minImageCount = swapchain_length;
        swapchain_info.imageFormat = swapchain_format.format;
        swapchain_info.imageColorSpace = swapchain_format.colorSpace;
        swapchain_info.imageArrayLayers = 1;
        
        if (graphics_family != present_family)
        {
            swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_info.queueFamilyIndexCount = 2;
            uint32_t indices[] = { graphics_family, present_family };
            swapchain_info.pQueueFamilyIndices = indices;
        }
        else
        {
            swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_info.queueFamilyIndexCount = 0;
            swapchain_info.pQueueFamilyIndices = nullptr;
        }

        swapchain_info.preTransform = surface_details.capabilities.currentTransform;
        swapchain_info.presentMode = swapchain_present_mode;
        swapchain_info.oldSwapchain = VK_NULL_HANDLE;

        return swapchain(swapchain_info, dev.vk());
    }

    swapchain::swapchain(VkSwapchainCreateInfoKHR create_info, VkDevice device)
        : vk_device(device), info(create_info)
    {
        VkResult result = vkCreateSwapchainKHR(vk_device, &create_info, nullptr, &vk_object);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create swapchain");

        uint32_t len = 0;
        vkGetSwapchainImagesKHR(vk_device, vk_object, &len, nullptr);
        images.resize(len);
        vkGetSwapchainImagesKHR(vk_device, vk_object, &len, images.data());
        image_views.resize(len);
        for (int i = 0; i < len; i++)
        {
            image_views[i] = image_view_builder(vk_device, images[i], info.imageFormat)
                .view_type(VK_IMAGE_VIEW_TYPE_2D)
                .aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
                .mip_levels(1)
                .base_mip_level(0)
                .base_array_layer(0)
                .layer_count(1)
                .build();
        }
    }
    void swapchain::destroy()
    {
        for (int i = 0; i < image_views.size(); i++)
            image_views[i].destroy();
        vkDestroySwapchainKHR(vk_device, vk_object, nullptr);
    }
}