#ifndef LAVA_RENDERER_H
#define LAVA_RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "typedef.h"

class lava_app;

class lava_renderer
{
public:
    lava_renderer(lava_app * app);

    void draw_frame();

    ~lava_renderer();

    void handle_window_resize(uint width, uint height);

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
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> inflight_fences;
    std::vector<VkFence> inflight_images;

    int current_frame;

    void create_swapchain();
    void create_image_views();
    void create_render_pass();
    void create_graphics_pipeline();
    void create_framebuffers();
    void create_command_pool();
    void create_command_buffers();
    void create_sync_objects();
    void destroy_swapchain();
    void recreate_swapchain();

    bool window_resized;
    uint window_width, window_height;
};

#endif