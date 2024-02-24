#pragma once
#include <GLFW/glfw3.h>

namespace Engine {

    extern GLFWwindow* window;

    void init();
    void loop();
    void cleanup();

}