#pragma once
#include "data.hpp"
#include "renderer.hpp"
#include <glm/ext/vector_int3.hpp>
struct SimpleMesh;
struct Chunk;

enum Direction: u32 {
    EAST = 0,
    SOUTH = 1,
    WEST = 2,
    NORTH = 3,
    UP = 4,
    DOWN = 5,
    DIRECTION_COUNT = 6
};
extern const Direction directionOpposite[DIRECTION_COUNT+1];
extern glm::ivec3 directionVector[DIRECTION_COUNT+1];

struct BlockModel {
    struct DrawInfo {
        SimpleMesh& mesh;
        Chunk& chunk;
        glm::ivec3 blockPos;
    };
    virtual void addToMesh(DrawInfo drawinfo) = 0;
};

struct NoModel : BlockModel {
    virtual void addToMesh(DrawInfo drawinfo) { (void)drawinfo; };
};
BlockModel* noModelConstructor(DataEntry* de);

struct CubeModel : BlockModel {
    u32 faces[6] = {};
    virtual void addToMesh(DrawInfo drawinfo);
};
BlockModel* cubeModelConstructor(DataEntry* de);

typedef BlockModel* (*BlockModelConstructor)(DataEntry* de);

struct Block {
    u8 solidity;
    BlockModel* model;
    string name;
    Block(string blockName, DataEntry* de);
};

struct Chunk {
    static constexpr u32 CHUNKSIZE = 32;
    typedef u8 blockID; 
    blockID blocks[CHUNKSIZE*CHUNKSIZE*CHUNKSIZE];
    
    void makeRandom();

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