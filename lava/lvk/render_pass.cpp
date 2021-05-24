#include "render_pass.h"
#include <algorithm>
#include <iterator>

namespace lvk
{
    render_pass_builder::render_pass_builder()
    {
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    }
    
    render_pass_builder & render_pass_builder::attachment(uint32_t index, attachment::type type, VkFormat format, VkSampleCountFlagBits samples)
    {
        VkAttachmentDescription description = {};
        VkAttachmentReference reference = {};
        reference.attachment = index;

        description.format = format;
        description.samples = samples;
        description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (type)
        {
        case attachment::color:
            description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        case attachment::depth:
            description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case attachment::resolve:
            description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        }
        attachments[index] = description;
        attachment_refs[index] = reference;
        return *this;
    }
    render_pass_builder & render_pass_builder::subpass(uint32_t index, pipeline::bind_point bind_point, std::vector<uint32_t> color_indices, std::vector<uint32_t> depth_indices, std::vector<uint32_t> resolve_indices)
    {
        subpass_description description = {};
        description.pipeline_bind_point = bind_point;

        description.color_references.resize(color_indices.size());
        description.depth_references.resize(depth_indices.size());
        description.resolve_references.resize(resolve_indices.size());

        for (size_t i = 0; i < color_indices.size(); i++)
        {
            description.color_references[i] = attachment_refs[color_indices[i]];
            description.depth_references[i] = attachment_refs[depth_indices[i]];
            if (!resolve_indices.empty())
                description.resolve_references[i] = attachment_refs[resolve_indices[i]];
        }
        subpasses[index] = description;
        return *this;
    }
    render_pass_builder & render_pass_builder::subpass_dependency(uint32_t src_subpass, uint32_t dst_subpass, pipeline::stage src_stages, access src_access, pipeline::stage dst_stages, access dst_access)
    {
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = src_subpass;
        dependency.dstSubpass = dst_subpass;
        dependency.srcStageMask = (VkPipelineStageFlagBits)src_stages;
        dependency.srcAccessMask = (VkAccessFlagBits)src_access;
        dependency.dstStageMask = (VkPipelineStageFlagBits)dst_stages;
        dependency.dstAccessMask = (VkAccessFlagBits)dst_access;
        subpass_dependencies.emplace_back(dependency);
        return *this;
    }
    render_pass render_pass_builder::build(const device & device)
    {
        VkRenderPassCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        auto ordered_attachments = collapse_indexed_map(attachments);
        auto ordered_attachment_refs = collapse_indexed_map(attachment_refs);
        
        auto ordered_lvk_subpasses = collapse_indexed_map(subpasses);
        std::vector<VkSubpassDescription> ordered_subpasses;
        std::transform(ordered_lvk_subpasses.begin(), 
                       ordered_lvk_subpasses.end(), 
                       std::back_inserter(ordered_subpasses), 
                       [](const subpass_description & v) {return v.unwrap(); });

        create_info.attachmentCount = ordered_attachments.size();
        create_info.pAttachments = ordered_attachments.data();
        create_info.subpassCount = ordered_subpasses.size();
        create_info.pSubpasses = ordered_subpasses.data();
        create_info.dependencyCount = subpass_dependencies.size();
        create_info.pDependencies = subpass_dependencies.data();

        return render_pass(device.vk(), create_info);
    }


    template <typename T>
    std::vector<T> render_pass_builder::collapse_indexed_map(std::map<uint32_t, T> map)
    {
        std::vector<T> result;
        for (size_t i = 0; i < map.size(); i++)
            if (map.find(i) != map.end())
                result.emplace_back(map[i]);
            else
                result.emplace_back(T{});
        return result;
    }


    render_pass::render_pass(VkDevice device, VkRenderPassCreateInfo create_info)
    {
        if (vkCreateRenderPass(device, &create_info, nullptr, &vk_object) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass");
    }
}