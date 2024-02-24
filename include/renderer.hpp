#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

struct Texture {
    u32 id;
    u32 index;
    i32 width;
    i32 height;
    i32 channels;
    void bind();
};

struct Shader {
    u32 id;
    Shader(const string& vertexSource, const string& fragmentSource);
    void bind();
    void setTexture(const char* name, const Texture& texture);
    void setFloat(const char* name, f32 value);
    void setVec3(const char* name, const glm::vec3& value);
    void setMat4(const char* name, const glm::mat4& value);
};

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

namespace gl {
    u32 compileShader(const string& shaderCode, u16 shaderType);
    u32 linkShaders(u32 vertexShader, u32 fragmentShader);
    u32 generateVBO(void* values, u32 size);
    u32 generateEBO(const vector<u32>& values);
    u32 generateVAO();
    //u32 addAttribToVAO(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    void addAttribToVAO(u32 index, i32 size, u32 type, i32 stride, u32 offset);
    Texture textureFromFile(const char* filename, u32 textureIndex = 0, u32 channels = 3);
};

struct Renderer {
    GLFWwindow* window;
    i32 width;
    i32 height;
    const char* title;
    static Renderer* instance;
    vector<Shader> shaders;
    vector<Texture> textures;
    SimpleMesh testmesh;
    bool disabledCursor = true;
    glm::vec2 mousePos = {0.0f, 0.0f};
    glm::vec2 cameraAngle = { 0.0f, 0.0f };
    glm::vec3 cameraPos = { 0, 0, 3.0f };

    Renderer() { instance = this; }
    void toggleCursor();

    void init(i32 width, i32 height, const char* title);
    void run();
    void destroy();

};