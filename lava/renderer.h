#ifndef LAVA_RENDERER_H
#define LAVA_RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "typedefs.h"
#include "lvk/instance.h"
#include "lvk/device.h"

struct SDL_Window;

namespace lvk
{
    struct DeviceSurfaceDetails;
}

namespace lava
{
    class App;

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texcoord;

        static VkVertexInputBindingDescription get_binding_description();
        static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions();

        bool operator==(const Vertex & other) const { return position == other.position && color == other.color && texcoord == other.texcoord; }
    };

    struct UniformBufferObject
    {
        glm::mat4 transform;
        glm::mat4 view;
        glm::mat4 proj;
        float hue_shift;
    };

    class Renderer
    {
    public:
        Renderer(App * app);

        void draw_frame();

        ~Renderer();

        void handle_window_resize();

    private:
        lvk::instance lvk_instance;
        VkInstance vulkan_instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        lvk::device lvk_device;
        VkDevice device;
        VkQueue graphics_queue;
        VkQueue present_queue;
        VkSurfaceKHR window_surface;
        VkSwapchainKHR swapchain;
        VkFormat swapchain_image_format;
        VkExtent2D swapchain_extent;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;
        std::vector<VkFramebuffer> swapchain_framebuffers;
        VkRenderPass render_pass;
        VkDescriptorSetLayout descriptor_set_layout;
        VkPipelineLayout pipeline_layout;
        VkPipeline graphics_pipeline;
        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;
        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_memory;
        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;
        uint32_t mip_levels;
        VkImage texture_image;
        VkDeviceMemory texture_image_memory;
        VkImageView texture_image_view;
        VkSampler texture_sampler;

        VkImage depth_image;
        VkDeviceMemory depth_image_memory;
        VkImageView depth_image_view;

        VkSampleCountFlagBits msaa_samples;

        std::vector<VkBuffer> uniform_buffers;
        std::vector<VkDeviceMemory> uniform_buffers_memory;

        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkFence> inflight_fences;
        std::vector<VkFence> inflight_images;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        VkImage color_image;
        VkDeviceMemory color_image_memory;
        VkImageView color_image_view;

        int current_frame;

        VkPhysicalDevice select_optimal_physical_device(const std::vector<VkPhysicalDevice> & physical_devices);

        void create_swapchain(lvk::DeviceSurfaceDetails surface_details);
        void create_image_views();
        void create_render_pass();
        void create_descriptor_set_layout();
        void create_graphics_pipeline();
        void create_framebuffers();
        void create_command_pool();
        void create_color_resources();
        void create_depth_resources();
        void create_texture_image();
        void create_texture_image_view();
        void create_texture_sampler();
        void load_model();
        void create_vertex_buffer();
        void create_index_buffer();
        void create_uniform_buffers();
        void create_descriptor_pool();
        void create_descriptor_sets();
        void create_command_buffers();
        void create_sync_objects();
        void destroy_swapchain();
        void recreate_swapchain();

        void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * memory);
        void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
        void create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits sample_count, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * memory);
        VkCommandBuffer begin_single_time_commands();
        void end_single_time_commands(VkCommandBuffer command_buffer);
        void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
        void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels);
        VkFormat find_supported_format(const std::vector<VkFormat> & candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat find_depth_format();
        void generate_mipmaps(VkImage image, VkFormat format, int32_t width, int32_t height, uint32_t mip_levels);

        void update_uniform_buffer(uint32_t current_image);

        SDL_Window * sdl_window;
        bool window_resized;
    };
}

#endif