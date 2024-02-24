#pragma once
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan_core.h>

namespace Engine {

    namespace SimplePipeline {

        struct Vertex {
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec2 texCoord;

            static VkVertexInputBindingDescription getBindingDescription();
            static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
        };

        struct SceneUBO {
            // do alignas(16) for mat4s
            glm::mat4 view;
            glm::mat4 proj;
            float time;

            void update();
        };

        struct ModelUBO {
            glm::mat4 model;

            void update();
        };

    };

};