#include <cmath>
#include <cstring>
#include <glad/glad.h>
#include "renderer.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

Renderer* Renderer::instance = nullptr;

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

Texture gl::textureFromFile(const char *filename, u32 textureIndex, u32 channels) {
    Texture t;
    u8* data = stbi_load(filename, &t.width, &t.height, &t.channels, channels); 
    if(data == nullptr) ERR_EXIT("Could not load image");
    glGenTextures(1, &t.id);
    t.index = textureIndex;
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, t.id);
    glTexImage2D(GL_TEXTURE_2D, 0, channels == 3 ? GL_RGB : GL_RGBA, t.width, t.height, 0, channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return t;
}

void Texture::bind() {
    glActiveTexture(index);
    glBindTexture(GL_TEXTURE_2D, id);
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

void Shader::setTexture(const char* name, const Texture& tex) {
    glUniform1i(glGetUniformLocation(this->id, name), tex.index);
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
    vertices.resize(4);
    f32 verticesRaw[] = {
       // positions                        // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    // top left 
       // positions                        // colors           // texture coords
        0.5f,  0.0f, 0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.0f, 0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.0f, 0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.0f, 0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    // top left 
       // positions                        // colors           // texture coords
        0.0f, 0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.0f, 0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.0f, 0.5f, -0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.0f, 0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    // top left
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
    glm::mat4 m = glm::scale(
        glm::translate(glm::mat4(), position),
        scaling);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
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
    Renderer::instance->width = width;
    Renderer::instance->height = height;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void) window;
    Renderer::instance->mousePos.x = xpos;
    Renderer::instance->mousePos.y = ypos;
}

void Renderer::toggleCursor() {
    disabledCursor = !disabledCursor;
    glfwSetInputMode(window, GLFW_CURSOR, disabledCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
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
    glfwSetInputMode(window, GLFW_CURSOR, disabledCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        ERR_EXIT("GLAD failed to load OpenGL functions");
    }
    
    glViewport(0, 0, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_set_flip_vertically_on_load(true);

    shaders.push_back(Shader(
        readFileString("shaders/simple.vert.glsl"), 
        readFileString("shaders/simple.frag.glsl")
    ));

    textures.push_back(gl::textureFromFile("textures/statue.jpg", 0, 3));
    textures.push_back(gl::textureFromFile("textures/tree.png", 1, 3));

    testmesh.loadTestData();
    testmesh.makeObjects();
}

void Renderer::run() {
    glfwSetTime(0);
    f32 ptime = 0.0;
    glm::vec2 lastMousePos = mousePos;
    bool prevEscape = false;

    while(!glfwWindowShouldClose(window)) {

        f32 time = glfwGetTime();
        f32 dt = time - ptime;
        ptime = time;

        cout << "CAMERA POS: " << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << "\n";
        cout << "CAMERA ANGLE: az=" << cameraAngle.x << " h=" << cameraAngle.y << "\n";

        glm::vec3 movement = {0, 0, 0};
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            movement += glm::vec3(0, 1, 0);
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            movement += glm::vec3(-1, 0, 0);
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            movement += glm::vec3(0, -1, 0);
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            movement += glm::vec3(1, 0, 0);
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            movement += glm::vec3(0, 0, 1);
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            movement += glm::vec3(0, 0, -1);
        bool escape = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        if(escape && !prevEscape)
            toggleCursor();
        prevEscape = escape;
        
        glm::vec3 movementReal = movement.x * glm::vec3(std::cos(cameraAngle.x), std::sin(cameraAngle.x), 0)
            + movement.y * glm::vec3(-std::sin(cameraAngle.x), std::cos(cameraAngle.x), 0)
            + movement.z * glm::vec3(0, 0, 1);
        constexpr float cameraAngleSpeed = 2.0f;
        constexpr float cameraSpeed = 0.6f;
        movementReal *= -dt * cameraSpeed; 
        cameraPos += movementReal;
        
        // update movement
        glm::vec2 mc = mousePos - lastMousePos;
        mc.x /= width;
        mc.y /= height;
        lastMousePos = mousePos;
        // / std::cos(cameraAngle.y)
        cameraAngle += glm::vec2(mc.x, mc.y) * cameraAngleSpeed;

        constexpr float PI = 3.14159268f;
        if(cameraAngle.y > 0.5f*PI)
            cameraAngle.y = 0.5f*PI;
        if(cameraAngle.y < -0.5f*PI)
            cameraAngle.y = -0.5f*PI;
        if(cameraAngle.x > PI)
            cameraAngle.x -= 2.0*PI;
        if(cameraAngle.x < -PI)
            cameraAngle.x += 2.0*PI;
        
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, cameraAngle.x, glm::vec3(0, 0, 1));
        view = glm::rotate(view, cameraAngle.y, glm::vec3(1, 0, 0));
        view = glm::inverse(view);
        view = glm::translate(view, -cameraPos);
        
        glm::mat4 proj = glm::perspective(70.0f/180.0f*PI, (f32)width/(f32)height, 0.1f, 100.0f);
    
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 

        glm::vec4 q = proj * view * model * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
        cout << q.x << " " << q.y << " " << q.z << " " << q.w << "\n";

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaders[0].bind();
        shaders[0].setFloat("time", time);
        shaders[0].setTexture("tex", textures[1]);
        shaders[0].setMat4("view", view);
        shaders[0].setMat4("proj", proj);
        testmesh.updateUniforms(shaders[0]);

        testmesh.draw();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Renderer::destroy() {
    glfwTerminate();
}

