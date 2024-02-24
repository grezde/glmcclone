#include "base.hpp"
#include <glad/glad.h>
#include "renderer.hpp"
#include <iostream>
#include <stb_image.h>

int main(int argc, const char** argv) {
    (void) argc;
    (void) argv;
    
    Renderer r;
    r.init(800, 600, "Hello warld");
    r.run();
    r.destroy();
    return 0;
}