#pragma once
#include "base.hpp"
#include "engine/pipeline.hpp"
#include "engine/scene.hpp"
#include <vulkan/vulkan_core.h>

namespace Engine {

    struct Store {

        vector<RenderPass> renderPasses;
        vector<Pipeline> pipelines;
        vector<Image> textures;
        vector<VkDescriptorSetLayout> dsLayouts;
        

    };

    extern Store store;

}