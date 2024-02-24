#pragma once
#include <vulkan/vulkan_core.h>

namespace Engine {

    struct CommandBuffer;

    struct RenderPass {

        VkRenderPass handle;

        // Creates the render pass
        void create();

        // Write to a command buffer that we enter the render stage
        void begin(CommandBuffer* cmdbuf, VkFramebuffer framebuffer, VkExtent2D extent);
        void end(CommandBuffer* cmdbuf);

        // Destroys the render pass
        void destroy();

    };

    // TODO: allow multiple render passes
    extern RenderPass renderpass;

}