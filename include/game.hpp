#pragma once
#include "renderer.hpp"
#include "world.hpp"
#include <fstream>

namespace Game {
    extern bool inspectMode;
    extern bool printFPS;
    extern World testWorld;
    extern glm::vec2 cameraAngle;
    extern glm::vec3 cameraPos;
    extern glm::vec3 cameraVel;

    void init();
    void run();
    void destory();

    void processInput(f32 dt);
};