#include "renderer.h"

#include <set>
#include <SDL/SDL_video.h>

#include "app.h"
#include "lvk.h"

#include <fstream>

static std::vector<char> load_file(const std::string filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open " + filename);

    size_t size = (size_t)file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT * callback_data,
                                                     void * user_data)
{
    printf("%s\n", callback_data->pMessage);
    return VK_FALSE;
}

lava_renderer::lava_renderer(lava_app * app)
{
    // Register application information
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app->title;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    // Query extensions needed by SDL
    uint32_t sdl_required_extension_count;
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, nullptr);
    std::vector<const char *> requested_extensions(sdl_required_extension_count);
    SDL_Vulkan_GetInstanceExtensions(app->sdl_window, &sdl_required_extension_count, requested_extensions.data());
    std::vector<const char *> requested_layers;

#if USE_VALIDATION
    requested_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    requested_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    auto extensions = lvk::filter_supported_extensions(requested_extensions);
    auto layers = lvk::filter_supported_layers(requested_layers);
    
    // Create Vulkan instance
    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = extensions.size();
    instance_create_info.ppEnabledExtensionNames = extensions.data();
    instance_create_info.enabledLayerCount = layers.size();
    instance_create_info.ppEnabledLayerNames = layers.data();
    instance_create_info.pNext = nullptr;

#if USE_VALIDATION
    auto debug_create_info = lvk::make_default_debug_messenger_create_info();
    debug_create_info.pfnUserCallback = debug_callback;
    instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
#endif
    if (vkCreateInstance(&instance_create_info, nullptr, &vulkan_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }
    
#if USE_VALIDATION
    debug_messenger = lvk::make_debug_messenger(vulkan_instance, &debug_create_info);
#endif

    if (!SDL_Vulkan_CreateSurface(app->sdl_window, vulkan_instance, &window_surface))
        throw std::runtime_error("Failed to create window surface from SDL!");

    // Find and select vulkan compatible device for use
    uint32_t device_count;
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);
    if (device_count == 0)
        throw std::runtime_error("Couldn't find any GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, physical_devices.data());

    physical_device = VK_NULL_HANDLE;
    lvk::QueueFamilyInfo queue_family_info = {};
    std::vector<const char *> supported_device_extensions = {};
    // Select a GPU which has the following properties and features...
    for (const auto & device : physical_devices)
    {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);

        queue_family_info = lvk::get_queue_family_info(device, window_surface);
        if (queue_family_info.graphics_family != LVK_NULL_QUEUE_FAMILY)
        {
            supported_device_extensions = lvk::filter_supported_device_extensions(device, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
            if (!supported_device_extensions.empty())
            {
                lvk::DeviceSurfaceDetails surface_details = lvk::query_surface_details(device, window_surface);
                if (!surface_details.formats.empty() && !surface_details.present_modes.empty())
                {
                    physical_device = device;
                    break;
                }
            }
        }
    }

    if (physical_device == VK_NULL_HANDLE)
        throw std::runtime_error("Couldn't find a GPU with appropriate features!");

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<int> unique_families = { queue_family_info.graphics_family, queue_family_info.present_family };
    float priority = 1.0f;

    for (int family : unique_families)
    {
        VkDeviceQueueCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = family;
        create_info.queueCount = 1;
        create_info.pQueuePriorities = &priority;
        queue_create_infos.push_back(create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = (uint32_t)queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = (uint32_t)supported_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = supported_device_extensions.data();

#if DEVICE_VALIDATION_LAYER_COMPATIBILITY
    device_create_info.enabledLayerCount = (uint32_t)layers.size();
    device_create_info.ppEnabledLayerNames = layers.data();
#else
    // Device specific validation layers are deprecated as of Vulkan 1.2
    device_create_info.enabledLayerCount = 0; 
#endif

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    vkGetDeviceQueue(device, queue_family_info.graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, queue_family_info.present_family, 0, &present_queue);

    lvk::DeviceSurfaceDetails swapchain_support = lvk::query_surface_details(physical_device, window_surface);
    VkSurfaceFormatKHR swapchain_format = lvk::choose_swapchain_surface_format(swapchain_support.formats);
    VkPresentModeKHR swapchain_present_mode = lvk::choose_swapchain_present_mode(swapchain_support.present_modes);
    VkExtent2D extent = lvk::choose_swapchain_extent(swapchain_support.capabilities, app->window_width, app->window_height);

    uint32_t swapchain_length = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        swapchain_length > swapchain_support.capabilities.maxImageCount)
    {
        swapchain_length = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = window_surface;
    swapchain_create_info.minImageCount = swapchain_length;
    swapchain_create_info.imageFormat = swapchain_format.format;
    swapchain_create_info.imageColorSpace = swapchain_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    if (queue_family_info.graphics_family != queue_family_info.present_family)
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        uint32_t indices[] = { queue_family_info.graphics_family, queue_family_info.present_family };
        swapchain_create_info.pQueueFamilyIndices = indices;
    }
    else
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    swapchain_create_info.preTransform = swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_length, nullptr);
    swapchain_images.resize(swapchain_length);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_length, swapchain_images.data());

    swapchain_image_format = swapchain_format.format;
    swapchain_extent = extent;

    swapchain_image_views.resize(swapchain_images.size());
    for (int i = 0; i < swapchain_images.size(); i++)
    {
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swapchain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swapchain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &create_info, nullptr, &swapchain_image_views[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view!");
    }

    auto vert_shader_source = load_file("shaders/vert.spv");
    auto frag_shader_source = load_file("shaders/frag.spv");
    
    VkShaderModule vertex_shader = lvk::create_shader_module(device, vert_shader_source);
    VkShaderModule fragment_shader = lvk::create_shader_module(device, frag_shader_source);

    VkPipelineShaderStageCreateInfo vertex_stage_info = {};
    vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage_info.module = vertex_shader;
    vertex_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_stage_info = {};
    fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    fragment_stage_info.module = fragment_shader;
    fragment_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_stage_info, fragment_stage_info };

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = nullptr;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchain_extent.width;
    viewport.height = (float)swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state;
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkDynamicState dynamic_states[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");

    VkAttachmentDescription color_attachment = {};
    color_attachment.format = swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass");

    vkDestroyShaderModule(device, vertex_shader, nullptr);
    vkDestroyShaderModule(device, fragment_shader, nullptr);
}

lava_renderer::~lava_renderer()
{
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    for (const auto image_view : swapchain_image_views)
        vkDestroyImageView(device, image_view, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(vulkan_instance, window_surface, nullptr);
    vkDestroyDevice(device, nullptr);
#if USE_VALIDATION
    lvk::destroy_debug_messenger(vulkan_instance, debug_messenger);
#endif
    vkDestroyInstance(vulkan_instance, nullptr);
}