#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>

struct Renderer {
    GLFWwindow* window;
    i32 width;
    i32 height;
    const char* title;
    static Renderer* instance;

    Renderer() { instance = this; }

    static u32 compileShader(const string& shaderCode, u16 shaderType);
    static u32 linkShaders(u32 vertexShader, u32 fragmentShader);

    void init(i32 width, i32 height, const char* title);
    void run();
    void destroy();

};