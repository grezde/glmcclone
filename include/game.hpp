#pragma once
#include "renderer.hpp"

struct InputHandler {
    bool disabledCursor = true;
    glm::vec2 mousePos = {0.0f, 0.0f};

    bool isPressed(i32 key);
    void init();
    void toggleCursor();
    void processInput(f32 dt);
};

struct Chunk {
    static constexpr u32 CHUNKSIZE = 32;
    typedef u8 blockID;
    blockID blocks[CHUNKSIZE*CHUNKSIZE*CHUNKSIZE];

    void makeSimpleMesh(SimpleMesh& mesh);
};

struct Game {
    Renderer renderer;
    InputHandler input;
    static Game* instance;
    bool printFPS = true;
    Chunk testChunk;

    Game() { instance = this; }
    void init();
    void run();
    void destory();
};