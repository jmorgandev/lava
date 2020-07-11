#ifndef LAVA_RENDERER_H
#define LAVA_RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "typedef.h"

class lava_app;
struct SDL_Window;

namespace lvk
{
    struct DeviceSurfaceDetails;
}

class lava_renderer
{
public:
    lava_renderer(lava_app * app);

    void draw_frame();

    ~lava_renderer();

    void handle_window_resize();

private:
    VkInstance vulkan_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
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
    VkImage texture_image;
    VkDeviceMemory texture_image_memory;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> inflight_fences;
    std::vector<VkFence> inflight_images;

    int current_frame;

    void create_swapchain(lvk::DeviceSurfaceDetails surface_details);
    void create_image_views();
    void create_render_pass();
    void create_descriptor_set_layout();
    void create_graphics_pipeline();
    void create_framebuffers();
    void create_command_pool();
    void create_texture_image();
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
    void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * memory);
    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer command_buffer);
    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    
    void update_uniform_buffer(uint32_t current_image);

    SDL_Window * sdl_window;
    bool window_resized;
};

#endif