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

struct BlockRenderProps {
    typedef u8 textureID;
    textureID faces[6]; // east, south, west, north, up, down
    u8 solidity; // to be checked later 
};

struct Chunk {
    static constexpr u32 CHUNKSIZE = 32;
    static vector<BlockRenderProps> props;
    typedef u8 blockID; 
    blockID blocks[CHUNKSIZE*CHUNKSIZE*CHUNKSIZE];

    static u32 indexOf(glm::ivec3 inChunkCoords);
    static bool inBounds(glm::ivec3 inChunkCoords);
    void makeSimpleMesh(SimpleMesh& mesh);
};

struct Game {
    Renderer renderer;
    InputHandler input;
    static Game* instance;
    bool printFPS = false;
    Chunk testChunk;

    Game() { instance = this; }
    void init();
    void run();
    void destory();
};