#include "game.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include <GLFW/glfw3.h>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_int3.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/vector_float3.hpp>

bool Game::printFPS = false;
bool Game::inspectMode = true;
World Game::testWorld;
glm::vec3 Game::cameraPos = { 21.0f, 26.0f, 21.0f };
glm::vec2 Game::cameraAngle = { -3.141f*0.75f, 0 };
glm::vec3 Game::cameraVel = { 0, 0, 0 };

void Game::processInput(f32 dt) {
    static constexpr float cameraAngleSpeed = 2.0f;
    static constexpr float cameraSpeed = 8.0f;
    static bool prevEscape = false;
    static bool prevI = false;
    static glm::vec2 lastMousePos = glm::vec2(0, 0);
    static bool lastMouseFirst = true;

    glm::vec3 movement = {0, 0, 0};
    if(window::isPressed(GLFW_KEY_W))
        movement += glm::vec3(0, 0, 1);
    if(window::isPressed(GLFW_KEY_A))
        movement += glm::vec3(-1, 0, 0);
    if(window::isPressed(GLFW_KEY_S))
        movement += glm::vec3(0, 0, -1);
    if(window::isPressed(GLFW_KEY_D))
        movement += glm::vec3(1, 0, 0);
    if(window::isPressed(GLFW_KEY_UP))
        movement += glm::vec3(0, 1, 0);
    if(window::isPressed(GLFW_KEY_DOWN))
        movement += glm::vec3(0, -1, 0);
    bool escape = window::isPressed(GLFW_KEY_ESCAPE);
    if(escape && !prevEscape)
        window::toggleCursor();
    bool I = window::isPressed(GLFW_KEY_I);
    if(I && !prevI)
        inspectMode = !inspectMode;
    prevI = I; 
    prevEscape = escape;

    glm::vec3 movementReal = movement.z * glm::vec3(std::cos(cameraAngle.x), 0, std::sin(cameraAngle.x))
            + movement.y * glm::vec3(0, 1, 0)
            + movement.x * glm::vec3(-std::sin(cameraAngle.x), 0, std::cos(cameraAngle.x));
    movementReal *= dt * cameraSpeed;
    cameraPos += movementReal;

    if(window::disabledCursor) {
        glm::vec2 mc = window::mousePos - lastMousePos;
        if(lastMouseFirst && (mc.x != 0.0f || mc.y != 0.0f)) {
            lastMouseFirst = false;
            return;
        }
        mc.x /= window::width;
        mc.y /= window::height;
        cameraAngle += glm::vec2(mc.x, mc.y) * cameraAngleSpeed;
        constexpr float PI = 3.14159268f;
        if(cameraAngle.y > 0.5f*PI*0.99f)
            cameraAngle.y = 0.5f*PI*0.99f;
        if(cameraAngle.y < -0.5f*PI*0.99f)
            cameraAngle.y = -0.5f*PI*0.99f;
        if(cameraAngle.x > PI)
            cameraAngle.x -= 2.0*PI;
        if(cameraAngle.x < -PI)
            cameraAngle.x += 2.0*PI;
    }
    lastMousePos = window::mousePos;
}

void Game::init() {
    window::init(1200, 900, "Hello warld");
    Registry::init();
    testWorld.init();
    window::beginDrawing();
    window::endDrawing();
}

void Game::run() {
    glfwSetTime(0);
    f32 ptime = 0.0;
    f32 fpst = 0.0f;
    u32 fpsc = 0;
    cout << "START RUNNING\n";

    while(!glfwWindowShouldClose(window::window)) {

        f32 time = glfwGetTime();
        f32 dt = time - ptime;
        ptime = time;
        fpst += dt;
        fpsc++;
        if(fpst > 1.0f) {
            if(printFPS)
                cout << fpsc/fpst << " FPS\n";
            fpsc = 0;
            fpst = 0.0f;
        }

        glfwPollEvents();
        processInput(dt);
        
        testWorld.update(time, dt);

        window::beginDrawing();

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + glm::vec3(
            std::cos(cameraAngle.x)*std::cos(cameraAngle.y),
            std::sin(cameraAngle.y),
            std::sin(cameraAngle.x)*std::cos(cameraAngle.y)
        ), glm::vec3(0, 1, 0));
        
        glm::mat4 proj = glm::perspective(glm::radians(70.0f), (f32)window::width/(f32)window::height, 0.01f, 1000.0f);

        shader::bind(Registry::shaders["voxel"]);
        shader::setMat4("view", view);
        shader::setMat4("proj", proj);
        gl::bindTexture(Registry::glTextures["blockAtlas"].glid, 0);
        shader::setTexture("tex", 0);

        testWorld.draw();

        window::endDrawing();
    }

}

void Game::destory() {
    window::destroy();
}

