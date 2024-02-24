#pragma once
#include "base.hpp"
#include <vulkan/vulkan_core.h>

namespace Engine {

    struct CommandBuffer {
        enum FLAGS {
            ONE_TIME = 1<<0,
            RESET    = 1<<1,
        };

        u32 flags;
        VkCommandBuffer handle;

        // create a one-time use command buffer
        void create(bool oneTime);

        static void createMultiple(CommandBuffer* dest, u32 count, bool oneTime);

        // begin recording
        void beginRecording();

        // end recording
        void endRecording();

        // submit the command buffer to be executed (blocking)
        void submit(VkFence fence = VK_NULL_HANDLE);

        // destroys everything
        void destroy();
    };

    struct Buffer {

        VkBuffer buffer;
        VkDeviceMemory memory;
        u32 size;

        // Creates the buffer
        void create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

        // Copies the data from the void pointer into the buffer
        void copyFromData(void* data);

        // Copies the data from another buffer to this buffer
        void copyFromBuffer(Buffer* b);

        // Copies the data from the void pointer using a staging buffer first
        void copyFromDataStaged(void* data);

        // Destroys the buffer and its memory
        void destroy();

        static u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);

    };

    struct Image {
        VkImage image;
        VkImageView view;
        // TODO: use VMA
        VkDeviceMemory memory = VK_NULL_HANDLE;

        // Creates everything
        void create();

        // Create an image from a file
        void createTextureFromFile(const char* filename);
        
        // Create a sampler from the image
        VkSampler createSampler();

        // Creates only the VkImage
        void createImage(u32 width, u32 height, VkFormat format, 
            VkImageTiling tiling, VkImageUsageFlags usage, 
            VkMemoryPropertyFlags properties);

        // Creates only the VkImageView
        void createView(VkFormat format, VkImageAspectFlags aspectFlags);

        // Transitions the image layout from one to another
        void transitionImageLayout(CommandBuffer* cmd, VkImageLayout oldLayout, VkImageLayout newLayout);

        // Copies the image from a buffer
        void copyFromBuffer(CommandBuffer* cmd, Buffer* buffer, u32 width, u32 height);

        // Destroys everything in the image
        void destroy();

        // Only used when the VkImage is owned by something else (i.e. in the swapchain) 
        void destroyOnlyView();
    };

}