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
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    u32 indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    u32 VAO;
    glGenVertexArrays(1, &VAO);  
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0); 

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    u32 vertexShader = Renderer::compileShader(readFileString("shaders/simple.vert.glsl"), GL_VERTEX_SHADER);
    u32 fragmentShader = Renderer::compileShader(readFileString("shaders/simple.frag.glsl"), GL_FRAGMENT_SHADER);
    u32 shaderProgram = Renderer::linkShaders(vertexShader, fragmentShader);
    
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);


    cout << "Starting running\n";
    r.run();

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    r.destroy();
    return 0;
}