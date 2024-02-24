#pragma once
#include <glm/mat4x4.hpp>
#include "engine/buffer.hpp"
#include "engine/swapchain.hpp"

namespace Engine {

    struct UniformBuffer {
        static constexpr u32 SET_COUNT = Swapchain::MAX_FIF;
        Buffer buffer;
        void* data;
        VkDescriptorSet ds[SET_COUNT];

        template<typename T> inline void create() { create_raw(SET_COUNT*sizeof(T)); }

        template<typename T> inline T* getData(u32 index) { return &((T*)data)[index]; }

        // Binds the uniform buffer tot the command buffer
        void bind(CommandBuffer* cmdbuf, u32 index);

        // destroys the uniform buffer object
        void destroy();

        private:
        void create_raw(u32 size);

    };

    struct Model {

        u32 pipelineID;
        UniformBuffer modelUBO;
        Buffer vertexBuffer;
        Buffer indexBuffer;
        u32 indexCount;

        // creates a model from an obj file
        template<typename VertexT>
        void createFromOBJ(const char* filename);

        // destroys all buffers
        void destroy();

    };

    struct Scene {

        UniformBuffer sceneUBO;
        vector<Model> models;

        void create();

        void destroy();

    };

    extern Scene scene;

}