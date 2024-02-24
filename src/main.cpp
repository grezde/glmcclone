#include "base.hpp"
#include <glad/glad.h>
#include "renderer.hpp"
#include <iostream>

int main(int argc, const char** argv) {
    (void) argc;
    (void) argv;

    Renderer r;
    r.init(800, 600, "Hello warld");
    cout << Renderer::instance->width << "\n";

    f32 vertices[] = {
        // positions         // colors
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
    };
    vector<u32> indices = {
        0, 1, 3,
        1, 2, 3
    };

    using namespace gl;

    u32 vertexShader = compileShader(readFileString("shaders/simple.vert.glsl"), GL_VERTEX_SHADER);
    u32 fragmentShader = compileShader(readFileString("shaders/simple.frag.glsl"), GL_FRAGMENT_SHADER);
    u32 shaderProgram = linkShaders(vertexShader, fragmentShader);

    u32 VBO = generateVBO(vertices, sizeof(vertices));
    u32 VAO = generateVAO();
    addAttribToVAO(0, 3, GL_FLOAT, 6*sizeof(float), 0);
    addAttribToVAO(1, 3, GL_FLOAT, 6*sizeof(float), 3*sizeof(float));
    u32 EBO = generateEBO(indices);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    cout << "Starting running\n";
    r.run(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    r.destroy();
    return 0;
}