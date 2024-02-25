#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

// Base abstractions of OpenGL
namespace gl {
    u32 compileShader(const string& shaderCode, u16 shaderType);
    u32 linkShaders(u32 vertexShader, u32 fragmentShader);
    u32 generateVBO(void* values, u32 size);
    u32 generateEBO(const vector<u32>& values);
    u32 generateVAO();
    //u32 addAttribToVAO(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    void addAttribToVAO(u32 index, i32 size, u32 type, i32 stride, u32 offset);
    u32 textureFromFile(const char* filename, u32 channels = 3, u32 desiredTexureSlot = 0);
    void bindTexture(u32 texture, u32 slot);
};

struct Shader {
    u32 id;
    Shader(const string& vertexSource, const string& fragmentSource);
    void bind();
    void setTexture(const char* name, u32 slot);
    void setFloat(const char* name, f32 value);
    void setVec3(const char* name, const glm::vec3& value);
    void setMat4(const char* name, const glm::mat4& value);
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

#ifdef DEBUG
    void loadTestData();
#endif
    void makeObjects();
    void updateUniforms(Shader& s);
    void draw();

};

// Renderer implementation
struct Renderer {
    GLFWwindow* window;
    i32 width;
    i32 height;
    const char* title;
    vector<Shader> shaders;
    vector<u32> textures;
    vector<SimpleMesh> meshes;
    glm::vec2 cameraAngle = { 0.0f, 0.0f };
    glm::vec3 cameraPos = { 10.0f, 0, 10.0f };

    void init(i32 width, i32 height, const char* title);
    void render(f32 time, f32 dt);
    void destroy();

};