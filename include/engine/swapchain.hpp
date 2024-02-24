#pragma once
#include "engine/buffer.hpp"
#include <vulkan/vulkan_core.h>

namespace Engine {

    struct RenderPass;

    struct Swapchain {

        VkSwapchainKHR swapchain;
        VkExtent2D extent;

        Image depthImage;
        VkFormat depthImageFormat;
        
        VkFormat imageFormat;
        vector<Image> images;
        vector<VkFramebuffer> framebuffers;

        static constexpr u32 MAX_FIF = 2;
        u32 currentFIF = 0;
        bool framebufferResized = false;

        CommandBuffer commandBuffers[MAX_FIF];
        VkSemaphore imageAvailableSemaphores[MAX_FIF];
        VkSemaphore renderFinishedSemaphores[MAX_FIF];
        VkFence     inFlightFences[MAX_FIF];
        
        Buffer sceneBufs[MAX_FIF];
        Buffer modelBufs[MAX_FIF];

        // Function to create the swapchain
        void create();

        // Function to createthe framebuffers. Must be called after initialising the renderpass
        void createFrameBuffers(RenderPass* renderPass);

        // Draws a frame to the screen
        void drawFrame();

        // Destroy everything
        void destroy();

        // Recreate everything on resize
        void recreate(RenderPass* renderpass);
        
    };

    extern Swapchain swapchain;

}