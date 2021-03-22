#ifndef LVK_DESCRIPTOR_SET_LAYOUT_H
#define LVK_DESCRIPTOR_SET_LAYOUT_H

#include <vulkan/vulkan.h>
#include <vector>

namespace lvk
{
    class device;

    class descriptor_set_layout
    {
    public:
        descriptor_set_layout() = default;
        descriptor_set_layout(VkDescriptorSetLayoutCreateInfo create_info, VkDevice device);

        VkDescriptorSetLayout vk() { return vk_descriptor_set_layout; }
    private:
        VkDescriptorSetLayout vk_descriptor_set_layout = VK_NULL_HANDLE;
    };

    class descriptor_set_layout_builder
    {
    public:
        descriptor_set_layout_builder();

        descriptor_set_layout_builder & uniform_buffer(uint32_t binding, uint32_t count, VkShaderStageFlags stage_flags);
        descriptor_set_layout_builder & combined_image_sampler(uint32_t binding, uint32_t count, VkShaderStageFlags stage_flags);

        descriptor_set_layout_builder & layout_binding(VkDescriptorType type, uint32_t binding, uint32_t count, VkShaderStageFlags stage_flags);
        descriptor_set_layout_builder & layout_binding(VkDescriptorSetLayoutBinding vk_binding);

        descriptor_set_layout_builder & layout_bindings(std::vector<VkDescriptorSetLayoutBinding> vk_bindings);

        descriptor_set_layout build(const device & device);
    private:
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        VkDescriptorSetLayoutCreateInfo create_info = {};
    };
}

#endif