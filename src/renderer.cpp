#include <cmath>
#include <cstring>
#include <glad/glad.h>
#include "engine.hpp"
#include "renderer.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/vector_float3.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game.hpp"

u32 shader::currentShader = 0;
vec4 window::clearColor = { 0.2f, 0.3f, 0.3f, 1.0f };
GLFWwindow* window::window = nullptr;
i32 window::width = 0, window::height = 0;
const char* window::title = nullptr;
Camera camera = {
    .pos = {0,0,0},
    .angles = {0,0,0},
    .fov = 70,
    .nearAnFar = {0.1f, 1000.0f},
    .view={}, .proj={},
};

void gl::updateVBO(u32 VBO, void* values, u32 size) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, values, GL_STATIC_DRAW);
}

void gl::updateEBO(u32 EBO, const vector<u32>& values) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, values.size()*4, values.data(), GL_STATIC_DRAW);
}

u32 gl::generateVBO(void* values, u32 size) {
    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, values, GL_STATIC_DRAW);
    return VBO;
}

u32 gl::generateEBO(const vector<u32>& values) {
    u32 EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, values.size()*4, values.data(), GL_STATIC_DRAW);
    return EBO;
}

u32 gl::generateVAO() {
    u32 VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    return VAO;
}

void gl::drawVAO(u32 VAO, u32 count) {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void gl::addAttribToVAO(u32 index, i32 size, u32 type, i32 stride, u32 offset) {
    switch(type) {
        case GL_UNSIGNED_INT:
            glVertexAttribIPointer(index, size, type, stride, (void*)(u64)offset);
            break;
        case GL_FLOAT:
            glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)(u64)offset);
            break;
        default:
            ERR_EXIT("UNCUGHT CASE HERE");
            break;
    }
    glEnableVertexAttribArray(index);
}

u32 gl::compileShader(const string& shaderCode, u16 shaderType) {
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

u32 gl::linkShaders(u32 vertexShader, u32 fragmentShader) {
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

u32 gl::textureFromFile(const char *filename, u32 desiredChannels) {
    i32 width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    u8* data = stbi_load(filename, &width, &height, &channels, desiredChannels); 
    if(data == nullptr) ERR_EXIT("Could not load image " << filename);
    u32 texture = gl::textureFromMemory(data, width, height, desiredChannels);
    stbi_image_free(data);
    return texture;
}

u32 gl::textureFromMemory(u8* data, u32 width, u32 height, u32 channels) {
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, channels == 3 ? GL_RGB : GL_RGBA, width, height, 0, channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texture;
}

void gl::bindTexture(u32 texture, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);
}

u32 gl::makeProgram(const string& vertexSource, const string& fragmentSource) {
    using namespace gl;
    u32 vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    u32 fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    u32 id = linkShaders(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return id;
}

void shader::bind(u32 shader) {
    currentShader = shader;
    glUseProgram(shader);
}

void shader::setTexture(const char* name, u32 slot) {
    glUniform1i(glGetUniformLocation(currentShader, name), slot);
}

void shader::setFloat(const char* name, f32 value) {
    glUniform1f(glGetUniformLocation(currentShader, name), value);
}

void shader::setVec3(const char* name, const vec3& value) {
    glUniform3fv(glGetUniformLocation(currentShader, name), 1, glm::value_ptr(value));   
}

void shader::setMat4(const char* name, const mat4& value) {
    glUniformMatrix4fv(glGetUniformLocation(currentShader, name), 1, GL_FALSE, glm::value_ptr(value));
}

void shader::setIvec3(const char* name, const ivec3& value) {
    glUniform3iv(glGetUniformLocation(currentShader, name), 1, glm::value_ptr(value));
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void) window;
    glViewport(0, 0, width, height);
    window::width = width;
    window::height = height;
}

void window::init(i32 width, i32 height, const char *title) {
    window::width = width;
    window::height = height;
    window::title = title;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  
    //glfwWindowHint(GLFW_SAMPLES, 4);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if(window == nullptr) {
        glfwTerminate();
        ERR_EXIT("GLFW Window creation failed");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, disabledCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        ERR_EXIT("GLAD failed to load OpenGL functions");
    }

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); 
    glFrontFace(GL_CW);
    //glEnable(GL_MULTISAMPLE); 

}

void window::beginDrawing() {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void window::endDrawing() {
    glfwSwapBuffers(window);
}

void window::destroy() {
    glfwTerminate();
}

void Camera::makeMatrices() {
    vec3 forward_proj = { cosf(angles.x), 0, sinf(angles.x) };
    vec3 right = { -forward_proj.z, 0, forward_proj.x };
    vec3 forward = forward_proj * cosf(angles.y) + vec3(0, 1, 0) * sinf(angles.y);
    vec3 up = vec3(0, 1, 0) * cosf(angles.z) + right * sinf(angles.z);
    view = glm::lookAt(pos, pos+forward, up);
    proj = glm::perspective(glm::radians(fov), (f32)window::width/(f32)window::height, 0.01f, 1000.0f);
}

void Camera::setMatrices() {
    shader::setMat4("view", view);
    shader::setMat4("proj", proj);
}


void SimpleMesh::addAttribs() {
    gl::addAttribToVAO(0, 3, GL_FLOAT, 8*sizeof(float), 0);
    gl::addAttribToVAO(1, 3, GL_FLOAT, 8*sizeof(float), 3*sizeof(float));
    gl::addAttribToVAO(2, 2, GL_FLOAT, 8*sizeof(float), 6*sizeof(float));
}

void SimpleMesh::updateUniforms() {
   shader::setMat4("model", modelMatrix);
}

void VoxelMesh::addAttribs() {
    gl::addAttribToVAO(0, 1, GL_UNSIGNED_INT, sizeof(VoxelVertex), 0);
    gl::addAttribToVAO(1, 1, GL_UNSIGNED_INT, sizeof(VoxelVertex), 4);
}

void VoxelMesh::updateUniforms() {
    shader::setIvec3("chunkCoords", chunkCoords);
}

