#include <glad/glad.h>
#include "renderer.hpp"

Renderer* Renderer::instance = nullptr;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void) window;
    glViewport(0, 0, width, height);
    Renderer::instance->width = width;
    Renderer::instance->height = height;
}

u32 Renderer::compileShader(const string& shaderCode, u16 shaderType) {
    u32 shader = glCreateShader(shaderType);
    const char* src = (const char*)shaderCode.data();
    glShaderSource(shader, 1, &src, nullptr); 
    glCompileShader(shader);
    i32 success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        ERR_EXIT((shaderType == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex") << " Shader Compilation failed: " << infoLog);
    }
    return shader;
}

u32 Renderer::linkShaders(u32 vertexShader, u32 fragmentShader) {
    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    i32 success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        ERR_EXIT("Shader Linking failed: " << infoLog);
    }
    return shaderProgram;
}

void Renderer::init(i32 width, i32 height, const char* title) {
    this->width = width;
    this->height = height;
    this->title = title;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  
    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    this->window = window;
    if(window == nullptr) {
        glfwTerminate();
        ERR_EXIT("GLFW Window creation failed");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        ERR_EXIT("GLAD failed to load OpenGL functions");
    }
    glViewport(0, 0, width, height);
}

void Renderer::run() {
    while(!glfwWindowShouldClose(window)) {

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Renderer::destroy() {
    glfwTerminate();
}

