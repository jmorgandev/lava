#ifndef LVK_RENDER_PASS_H
#define LVK_RENDER_PASS_H

#include <map>

#include "object.h"
#include "device.h"

namespace lvk
{
    class render_pass : public object<VkRenderPass>
    {
    public:
        render_pass(VkDevice device, VkRenderPassCreateInfo create_info);
    private:

    };

    namespace attachment
    {
        enum type
        {
            color,
            depth,
            resolve
        };
    }

    namespace pipeline
    {
        enum bind_point
        {
            graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
            compute = VK_PIPELINE_BIND_POINT_COMPUTE
        };
        enum stage
        {
            color_attachment_output = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
    }
    enum class access
    {
        none = 0,
        color_attachment_write = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    struct subpass_description : wrapper<VkSubpassDescription>
    {
        pipeline::bind_point pipeline_bind_point;
        std::vector<VkAttachmentReference> color_references;
        std::vector<VkAttachmentReference> depth_references;
        std::vector<VkAttachmentReference> resolve_references;

        VkSubpassDescription unwrap() const override
        {
            VkSubpassDescription desc = {};
            desc.pipelineBindPoint = (VkPipelineBindPoint)pipeline_bind_point;
            desc.colorAttachmentCount = color_references.size();
            desc.pColorAttachments = color_references.data();
            desc.pDepthStencilAttachment = depth_references.data();
            if (!resolve_references.empty())
                desc.pResolveAttachments = resolve_references.data();
            return desc;
        }
    };

    class render_pass_builder
    {
    public:
        render_pass_builder();

        render_pass_builder & attachment(uint32_t index, attachment::type type, VkFormat format, VkSampleCountFlagBits samples);
        render_pass_builder & subpass(uint32_t index, pipeline::bind_point bind_point, std::vector<uint32_t> color_indices, std::vector<uint32_t> depth_indices, std::vector<uint32_t> resolve_indices);
        render_pass_builder & subpass_dependency(uint32_t src_subpass, uint32_t dst_subpass, pipeline::stage src_stages, access src_access, pipeline::stage dst_stages, access dst_access);

        render_pass build(const device & device);
    private:
        template <typename T>
        std::vector<T> collapse_indexed_map(std::map<uint32_t, T> map);

        std::map<uint32_t, VkAttachmentDescription> attachments;
        std::map<uint32_t, VkAttachmentReference> attachment_refs;
        std::map<uint32_t, subpass_description> subpasses;
        std::vector<VkSubpassDependency> subpass_dependencies;
        VkRenderPassCreateInfo create_info;
    };
}

#endif