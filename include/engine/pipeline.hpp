#pragma once
#include "base.hpp"
#include "engine/swapchain.hpp"
#include <vulkan/vulkan_core.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <array>

namespace Engine {

    struct RenderPass;
    struct CommandBuffer;

    struct Pipeline {

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        // UniformBuffer ubos[Swapchain::MAX_FIF];

        VkPipelineLayout layout;
        VkPipeline handle;

        void createUniformBuffers(Image texture, VkSampler sampler);

        // Creates the pipeline
        void create(RenderPass* renderPass);

        // Begin the command buffer
        void bind(CommandBuffer* cmdbuf);

        // Destroys the pipeline
        void destroy();
    };

    extern Pipeline pipeline;

}