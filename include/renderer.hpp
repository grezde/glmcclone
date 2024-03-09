#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_int3.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

// Base abstractions of OpenGL
namespace gl {
    u32 compileShader(const string& shaderCode, u16 shaderType);
    u32 linkShaders(u32 vertexShader, u32 fragmentShader);
    u32 makeProgram(const string& vertexSource, const string& fragmentSource);

    u32 generateVBO(void* values, u32 size);
    u32 generateEBO(const vector<u32>& values);
    u32 generateVAO();
    void addAttribToVAO(u32 index, i32 size, u32 type, i32 stride, u32 offset);

    u32 textureFromFile(const char* filename, u32 channels = 4);
    u32 textureFromMemory(u8* data, u32 width, u32 height, u32 channels = 4);
    void bindTexture(u32 texture, u32 slot);
};

// Shader values
namespace shader {
    extern u32 currentShader;
    void bind(u32 shader);
    void setTexture(const char* name, u32 slot);
    void setFloat(const char* name, f32 value);
    void setVec3(const char* name, const glm::vec3& value);
    void setMat4(const char* name, const glm::mat4& value);
    void setIvec3(const char* name, const glm::ivec3& value);
};


// GLFW window wrapper
namespace window {
    extern glm::vec4 clearColor;
    extern GLFWwindow* window;
    extern i32 width, height;
    extern const char* title;
    extern bool disabledCursor;
    extern glm::vec2 mousePos;
    
    void init(i32 width, i32 height, const char* title);
    bool isPressed(i32 key);
    void beginDrawing();
    void endDrawing();
    void toggleCursor();
    void destroy();
};

// Simple Shader Implementation

struct SimpleVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoords;
};

struct SimpleMesh {

    vector<SimpleVertex> vertices;
    vector<u32> indices;
    u32 VAO, VBO, EBO;
    glm::vec3 position = { 0, 0, 0 };
    //glm::vec3 rotation;
    glm::vec3 scaling = { 1, 1, 1 };

    void makeObjects();
    void updateUniforms();
    void draw();

};

// Voxel Shader implemetation

struct VoxelVertex {
    union {
        struct {
            u32 padding0 : 10;
            u32 ao : 4;
            u32 x : 6;
            u32 y : 6;
            u32 z : 6;
        };
        u32 pos_ao;
    };

    union {
        struct {
            u32 padding1 : 16;
            u32 u : 8;
            u32 v : 8;
        };
        u32 texCoords;
    };
};

struct VoxelMesh {

    vector<VoxelVertex> vertices;
    vector<u32> indices;
    u32 VAO, VBO, EBO;
    glm::ivec3 chunkCoords;

    void makeObjects();
    void updateUniforms();
    void draw();

};