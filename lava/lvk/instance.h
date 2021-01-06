#ifndef LVK_INSTANCE_H
#define LVK_INSTANCE_H
#include <vector>
#include <string>
#include <cstdint>
#include <vulkan/vulkan.h>
namespace lvk
{
    class instance;
    class instance_builder
    {
    public:
        instance_builder();
        instance_builder & application_name(const char * name);
        instance_builder & application_version(uint32_t major, uint32_t minor, uint32_t patch);
        instance_builder & engine_name(const char * name);
        instance_builder & engine_version(uint32_t major, uint32_t minor, uint32_t patch);
        instance_builder & api_version(uint32_t version);
        instance_builder & api_version(uint32_t major, uint32_t minor, uint32_t patch);
        instance_builder & extension(const char * name);
        instance_builder & extensions(const std::vector<const char *> & names);
        instance_builder & layer(const char * name);
        instance_builder & layers(const std::vector<const char *> & names);
        instance_builder & default_debug_messenger();
        instance_builder & custom_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT info);

        instance build();
    private:
        VkApplicationInfo app_info;
        VkInstanceCreateInfo create_info;
        VkDebugUtilsMessengerCreateInfoEXT debug_info;
        bool using_debug_messenger;
        std::vector<const char *> requested_extensions;
        std::vector<const char *> requested_layers;
    };

    class instance
    {
    public:
        instance() : vk_instance(0), vk_debug_messenger(0) {}
        instance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE);
        instance(instance && rhs);
        instance & operator=(instance && rhs);

        instance(const instance &) = delete;
        instance & operator=(const instance &) = delete;

        VkInstance get_vk_instance() { return vk_instance; }
        VkDebugUtilsMessengerEXT get_debug_messenger() { return vk_debug_messenger; }

        ~instance();
    private:
        VkInstance vk_instance;
        VkDebugUtilsMessengerEXT vk_debug_messenger;
    };
}
#endif