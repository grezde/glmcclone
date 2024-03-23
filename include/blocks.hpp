#pragma once
#include "base.hpp"
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_int3.hpp>

enum Direction: u32 {
    EAST = 0,
    SOUTH = 1,
    WEST = 2,
    NORTH = 3,
    UP = 4,
    DOWN = 5,
    DIRECTION_COUNT = 6
};
extern const Direction directionOpposite[DIRECTION_COUNT];
extern ivec3 directionVector[DIRECTION_COUNT];
extern const char* directionNames[DIRECTION_COUNT];
extern ivec2 directionToAxisAndSign[DIRECTION_COUNT];

struct DataEntry;
struct Chunk;
struct VoxelMesh;

struct BlockModel {
    struct DrawInfo {
        VoxelMesh& mesh;
        const Chunk& chunk;
        const Chunk* neighbours[DIRECTION_COUNT];
        ivec3 blockPos;
    };
    virtual void addToMesh(DrawInfo drawinfo) = 0;
};

struct NoModel : BlockModel {
    static BlockModel* constructor(DataEntry* de);
    virtual void addToMesh(DrawInfo drawinfo) { (void)drawinfo; };
};

struct CubeModel : BlockModel {
    u32 faces[6] = {};
    u8 permutions[6] = {};
    // represents the permutation of the uv coords of each face
    // first 2 bits: where the 0,0 uv coords are
    // the 1,1 must be on the opposite side
    // the 3rd bit: whether after 0,0 there is 0,1 or 1,0
    static u8 getPermutation(Direction facedir, Direction texdir, bool flip);
    
    static BlockModel* constructor(DataEntry* de);
    virtual void addToMesh(DrawInfo drawinfo);
};

struct Block {
    u8 solidity;
    BlockModel* model = nullptr;
    // for block variants, the naming convention is
    // log/facing_x <= log is the block, facing is the prop name and x is the enum value
    // or: bed/color_red/wood_oak
    // or: tiny_block/pallete_stone/fullness_4A
    // enums are defined in data
    string name;
    Block(string blockName, DataEntry* de);
};