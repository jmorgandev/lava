#include "descriptor_set_layout.h"
#include "device.h"

namespace lvk
{
    descriptor_set_layout_builder::descriptor_set_layout_builder()
    {
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    }
    descriptor_set_layout_builder & descriptor_set_layout_builder::layout_binding(VkDescriptorSetLayoutBinding vk_binding)
    {
        bindings.push_back(vk_binding);
        return *this;
    }
    descriptor_set_layout_builder & descriptor_set_layout_builder::layout_binding(VkDescriptorType type, uint32_t binding, uint32_t count, VkShaderStageFlags stage_flags)
    {
        VkDescriptorSetLayoutBinding vk_binding{};
        vk_binding.descriptorType = type;
        vk_binding.binding = binding;
        vk_binding.descriptorCount = count;
        vk_binding.stageFlags = stage_flags; 
        vk_binding.pImmutableSamplers = nullptr;
        return layout_binding(vk_binding);
    }
    descriptor_set_layout_builder & descriptor_set_layout_builder::layout_bindings(std::vector<VkDescriptorSetLayoutBinding> vk_bindings)
    {
        bindings.insert(bindings.end(), vk_bindings.begin(), vk_bindings.end());
        return *this;
    }
    descriptor_set_layout_builder & descriptor_set_layout_builder::uniform_buffer(uint32_t binding, uint32_t count, VkShaderStageFlags stage_flags)
    {
        return layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, count, stage_flags);
    }
    descriptor_set_layout_builder & descriptor_set_layout_builder::combined_image_sampler(uint32_t binding, uint32_t count, VkShaderStageFlags stage_flags)
    {
        return layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding, count, stage_flags);
    }
    descriptor_set_layout descriptor_set_layout_builder::build(const device & device)
    {
        create_info.bindingCount = (uint32_t)bindings.size();
        create_info.pBindings = bindings.data();

        return descriptor_set_layout(create_info, device.vk());
    }

    descriptor_set_layout::descriptor_set_layout(VkDescriptorSetLayoutCreateInfo create_info, VkDevice device)
    {
        VkResult result = vkCreateDescriptorSetLayout(device, &create_info, nullptr, &vk_descriptor_set_layout);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Could not create descriptor set layout");
    }
}