#pragma once
#include "renderer.hpp"

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

struct WorldChunk {
    glm::ivec3 coords;
    Chunk chunk;
    SimpleMesh mesh;
};

struct World {
    vector<WorldChunk> chunks;
    void init();
    void draw();
};

struct Game {
    Renderer renderer;
    static Game* instance;
    bool printFPS;
    World testWorld;
    Chunk testChunk;

    Game() { instance = this; }
    void processInput(f32 dt);
    void init();
    void run();
    void destory();
};