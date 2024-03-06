#pragma once
#include "renderer.hpp"
#include "world.hpp"

namespace Game {
    extern bool printFPS;
    extern World testWorld;
    extern glm::vec2 cameraAngle;
    extern glm::vec3 cameraPos;

    void init();
    void run();
    void destory();

    void processInput(f32 dt);
};