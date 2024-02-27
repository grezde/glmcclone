#include <cmath>
#include <cstring>
#include <glad/glad.h>
#include "renderer.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/vector_float3.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game.hpp"

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

void gl::addAttribToVAO(u32 index, i32 size, u32 type, i32 stride, u32 offset) {
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)(u64)offset);
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
    u8* data = stbi_load(filename, &width, &height, &channels, desiredChannels); 
    if(data == nullptr) ERR_EXIT("Could not load image");
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

void Shader::bind() {
    glUseProgram(id);
}

Shader::Shader(const string& vertexSource, const string& fragmentSource) {
    using namespace gl;
    u32 vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    u32 fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    id = linkShaders(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::setTexture(const char* name, u32 slot) {
    glUniform1i(glGetUniformLocation(this->id, name), slot);
}

void Shader::setFloat(const char* name, f32 value) {
    glUniform1f(glGetUniformLocation(this->id, name), value);
}

void Shader::setVec3(const char* name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(this->id, name), 1, glm::value_ptr(value));   
}

void Shader::setMat4(const char* name, const glm::mat4& value) {
    glUniformMatrix4fv(glGetUniformLocation(this->id, name), 1, GL_FALSE, glm::value_ptr(value));
}

#ifdef DEBUG
void SimpleMesh::loadTestData() {
    vertices.resize(12);
    f32 verticesRaw[] = {
       // positions                        // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    // top left

        0.5f,  -0.5f+0.0f,  0.5f+0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f,  -0.5f+0.0f,  0.5f+-0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f+ 0.0f, 0.5f+-0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f, -0.5f+ 0.0f, 0.5f+ 0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    // top left

        0.5f+0.0f,  0.5f, 0.5f+ 0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f+0.0f, -0.5f, 0.5f+ 0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        0.5f+0.0f, -0.5f, 0.5f+-0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        0.5f+0.0f,  0.5f, 0.5f+-0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    // top left
    };
    memcpy(vertices.data(), verticesRaw, sizeof(verticesRaw));
    indices = {
        0, 1, 3,
        1, 2, 3,
        
        0+4, 1+4, 3+4,
        1+4, 2+4, 3+4,

        0+8, 1+8, 3+8,
        1+8, 2+8, 3+8,
    };
}
#endif

void SimpleMesh::makeObjects() {
    using namespace gl;
    VBO = generateVBO(vertices.data(), vertices.size()*sizeof(*vertices.data()));
    VAO = generateVAO();
    addAttribToVAO(0, 3, GL_FLOAT, 8*sizeof(float), 0);
    addAttribToVAO(1, 3, GL_FLOAT, 8*sizeof(float), 3*sizeof(float));
    addAttribToVAO(2, 2, GL_FLOAT, 8*sizeof(float), 6*sizeof(float));
    EBO = generateEBO(indices);
}

void SimpleMesh::updateUniforms(Shader& s) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, scaling);
    model = glm::translate(model, position);
    s.setMat4("model", model);
}

void SimpleMesh::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    //glDrawArrays(GL_TRIANGLES, 0, 3);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void) window;
    glViewport(0, 0, width, height);
    Game::instance->renderer.width = width;
    Game::instance->renderer.height = height;
}

void Renderer::makeAtlas() {
    // u32 is the color
    static constexpr u32 TILE = 16;
    u32 width = 8, height=8;
    u32* data = (u32*)malloc(4*TILE*TILE*width*height);
    memset(data, 0, 4*TILE*TILE*width*height);
    for(u32 y=0; y<TILE; y++) for(u32 x=0; x<TILE; x++) {
        u32 mag = (x < 8) ^ (y < 8);
        data[y*TILE*width+x] = mag ? 0xFFFF00FF : 0xFF000000;
    }
    for(u32 i=1; i<10; i++) {
        char filename[30];
        sprintf(filename, "textures/%u.png", i);
        i32 imageWidth, imageHeight, imageChannels;
        u8* newdata = stbi_load(filename, &imageWidth, &imageHeight, &imageChannels, 4);
        if(data == nullptr) ERR_EXIT("Could not load image " << i);
        cout << filename << ": " << imageChannels << " " << imageWidth << " " << imageHeight << "\n";
        if(imageChannels != 4 || imageWidth != TILE || imageHeight != TILE)
            ERR_EXIT("Image not in apropriate format " << i);
        for(u32 y=0; y < TILE; y++)
        for(u32 x=0; x < TILE; x++) {
            data[((i/width)*TILE + y)*TILE*width + (i%width)*TILE + x] = ((u32*)newdata)[TILE*y+x];
        };
        stbi_image_free(newdata);
    }
    #ifdef DEBUG
        stbi_write_png("output/atlas.png", TILE*width, TILE*height, 4, data, TILE*width*4);
    #endif
    // Finally we must flip everything in acordance to OpenGL
    for(u32 y=0; y<TILE*height/2; y++)
    for(u32 x=0; x<TILE*width; x++) {
        u32 temp = data[y*TILE*width+x];
        data[y*TILE*width+x] = data[(height*TILE-y-1)*TILE*width+x];
        data[(height*TILE-y-1)*TILE*width+x] = temp;
    };
    textures.push_back(gl::textureFromMemory((u8*)data, width*TILE, height*TILE, 4));
    free(data);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaders.push_back(Shader(
        readFileString("shaders/simple.vert.glsl"), 
        readFileString("shaders/simple.frag.glsl")
    ));

    //textures.push_back(gl::textureFromFile("textures/statue.jpg", 3, 0));
    //textures.push_back(gl::textureFromFile("textures/tree.png", 3, 1));
    
    //meshes.push_back(SimpleMesh());
    //meshes[0].loadTestData();
    //meshes[0].makeObjects();
    //meshes[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    //
    //meshes.push_back(SimpleMesh());
    //meshes[1].loadTestData();
    //meshes[1].makeObjects();
    //meshes[1].position = glm::vec3(-2.0f, 0.0f, 0.0f);

    cameraPos = glm::vec3(0, 0, 3.0f);
    cameraAngle = glm::vec2(-3.141f/2.0f, 0);
}

void Renderer::render(f32 time, f32 dt) {
    (void) dt;
        
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + glm::vec3(
        std::cos(cameraAngle.x)*std::cos(cameraAngle.y),
        std::sin(cameraAngle.y),
        std::sin(cameraAngle.x)*std::cos(cameraAngle.y)
    ), glm::vec3(0, 1, 0));
    
    glm::mat4 proj = glm::perspective(glm::radians(70.0f), (f32)width/(f32)height, 0.1f, 100.0f);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders[0].bind();
    shaders[0].setMat4("view", view);
    shaders[0].setMat4("proj", proj);
    shaders[0].setFloat("time", time);
    gl::bindTexture(textures[0], 0);
    shaders[0].setTexture("tex", 0);
    
    for(SimpleMesh& mesh : meshes) {
        mesh.updateUniforms(shaders[0]);
        mesh.draw();
    }
    
    glfwSwapBuffers(window);    
}

void Renderer::destroy() {
    glfwTerminate();
}

