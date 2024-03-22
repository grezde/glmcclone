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
    void updateVBO(u32 VBO, void* values, u32 size);
    void updateEBO(u32 EBO, const vector<u32>& values);
    u32 generateVAO();
    void addAttribToVAO(u32 index, i32 size, u32 type, i32 stride, u32 offset);
    void drawVAO(u32 VAO, u32 count);

    u32 textureFromFile(const char* filename, u32 channels = 4);
    u32 textureFromMemory(u8* data, u32 width, u32 height, u32 channels = 4);
    void bindTexture(u32 texture, u32 slot);
};

// Shader values
namespace shader {
    extern u32 currentShader;
    void bind(u32 shader);
    // TODO: see how std::map implements a hash template and use the same thing to use either locations or uniform names for binding
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

template<typename VertexT>
struct Mesh {
    vector<VertexT> vertices;
    vector<u32> indices;
    u32 indicesCount = 0;
    u32 VAO, VBO, EBO;

    virtual void addAttribs() = 0;
    virtual void updateUniforms() = 0;

    void makeObjects() {
        using namespace gl;
        if(indicesCount != 0) {
            indicesCount = indices.size();
            updateVBO(VBO, vertices.data(), vertices.size()*sizeof(*vertices.data()));
            updateEBO(EBO, indices);
        }
        else {
            indicesCount = indices.size();
            VBO = generateVBO(vertices.data(), vertices.size()*sizeof(*vertices.data()));
            VAO = generateVAO();
            this->addAttribs();
            EBO = generateEBO(indices);
        }
        indices.clear(); indices.shrink_to_fit();
        vertices.clear(); vertices.shrink_to_fit();
    }

    void draw() {
        gl::drawVAO(VAO, indicesCount);
    }
};

struct SimpleVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoords;
};

struct SimpleMesh : Mesh<SimpleVertex> {

    glm::mat4 modelMatrix = glm::mat4(1);

    virtual void addAttribs();
    virtual void updateUniforms();

};

// Voxel Shader implemetation

struct VoxelVertex {
    u32 pos_ao;
    u32 texCoords;
};

struct VoxelMesh : Mesh<VoxelVertex> {

    glm::ivec3 chunkCoords;

    virtual void addAttribs();
    virtual void updateUniforms();

};