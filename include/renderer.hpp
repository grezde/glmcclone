#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>

namespace gl {
    u32 compileShader(const string& shaderCode, u16 shaderType);
    u32 linkShaders(u32 vertexShader, u32 fragmentShader);
    u32 generateVBO(void* values, u32 size);
    u32 generateEBO(const vector<u32>& values);
    u32 generateVAO();
    //u32 addAttribToVAO(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    void addAttribToVAO(u32 index, i32 size, u32 type, i32 stride, u32 offset);
};

struct Renderer {
    GLFWwindow* window;
    i32 width;
    i32 height;
    const char* title;
    static Renderer* instance;

    Renderer() { instance = this; }

    void init(i32 width, i32 height, const char* title);
    void run(u32 shaderProgram);
    void destroy();

};