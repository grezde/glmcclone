#pragma once
#include "base.hpp"
#include <vulkan/vulkan_core.h>

namespace Engine {

    // struct containing global Vulkan configuration
    struct Vulkan {

        // Useful information about a physical device
        struct PDInfo {
            u32 score;
            u32 graphicsQF;
            u32 presentQF;
        };

        VkAllocationCallbacks* allocator;
        VkInstance instance;
        VkDevice device;
        VkPhysicalDevice pdevice;
        VkSurfaceKHR surface;

        PDInfo pdinfo;        
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkCommandPool commandPool;
        
        #ifdef DEBUG
            VkDebugUtilsMessengerEXT debugMessenger;
        #endif

        void create();
        void destroy();

        static PDInfo PDGetInfoFaster(VkPhysicalDevice pd, VkSurfaceKHR surface);
        static PDInfo PDGetInfo(VkPhysicalDevice pd, VkSurfaceKHR surface, vector<const char*>& deviceExtensions);

    };

    extern Vulkan vulkan;

}