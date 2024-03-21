#pragma once
#include "data.hpp"
#include "renderer.hpp"
#include <cstdlib>
#include <glm/ext/vector_int3.hpp>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
struct VoxelMesh;
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
extern const Direction directionOpposite[DIRECTION_COUNT];
extern glm::ivec3 directionVector[DIRECTION_COUNT];
extern const char* directionNames[DIRECTION_COUNT];
extern glm::ivec2 directionToAxisAndSign[DIRECTION_COUNT];

struct AABB {
    glm::vec3 start;
    glm::vec3 size;
};

struct BlockModel {
    struct DrawInfo {
        VoxelMesh& mesh;
        const Chunk& chunk;
        const Chunk* neighbours[DIRECTION_COUNT];
        glm::ivec3 blockPos;
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

struct Chunk {
    static constexpr u32 CHUNKSIZE = 32;
    typedef u8 blockID; 
    blockID blocks[CHUNKSIZE*CHUNKSIZE*CHUNKSIZE];
    u16 lightlevels[CHUNKSIZE*CHUNKSIZE*CHUNKSIZE];

    void makeSin(glm::ivec3 coords);
    void makeRandom();

    static u32 indexOf(glm::ivec3 inChunkCoords);
    static bool inBounds(glm::ivec3 inChunkCoords);
    //void makeSimpleMesh(SimpleMesh& mesh);
    void makeVoxelMesh(VoxelMesh& mesh, Chunk* neighbours[6]) const;
};

typedef glm::ivec4 UUID;
inline UUID UUID_make() { return {rand(),rand(),rand(),rand()}; }

struct Entity {
    u32 type;
    UUID uuid;
    glm::vec3 pos, vel;
    glm::vec3 lookingAt;
    DataEntry* data;
    SimpleMesh mesh;
};

struct EntityModel {
    u32 texture;
    void setTexture(DataEntry* de);
    virtual void makeMesh(Entity* entity) = 0;
};

struct NoEntityModel : EntityModel {
    static EntityModel* constructor(DataEntry*) { return new NoEntityModel(); }
    virtual inline void makeMesh(Entity*) {};
};

struct CuboidsEntityModel : EntityModel {
    
    struct VertexIntermediary {
        glm::vec3 xyz;
        glm::ivec2 uv;
    };

    struct Cuboid {
        glm::ivec2 uv;
        glm::ivec3 dimensions;

        void makeIntermediary(VertexIntermediary* vi);
        Cuboid(DataEntry* de);
    };

    struct AppliedCuboid {
        u32 id;
        glm::vec3 pos;
        Direction facing, downwards;
        bool flip;

        AppliedCuboid(const map<string, u32>& cuboidMap, DataEntry* de);
        void makeIntermediary(VertexIntermediary* vi);
    }; 

    struct Object {
        string name;
        glm::vec3 pivot;
        vector<AppliedCuboid> cuboids;
        Object(const map<string, u32>& cuboidMap, const string& name, DataEntry* de);
    };

    vector<Cuboid> cuboids;
    vector<Object> objects;

    CuboidsEntityModel(DataEntry* de);
    static EntityModel* constructor(DataEntry* de);
    virtual void makeMesh(Entity* entity);
};

struct EntityType {
    u32 id;
    string name;
    EntityModel* model;
    u32 movement;
    AABB aabb;
    EntityType(string entityName, DataEntry* de);
};

struct WorldChunk {
    glm::ivec3 coords;
    Chunk chunk;
    std::unordered_map<UUID, Entity*> entities;
    bool needsRemeshing;
    VoxelMesh mesh;
    WorldChunk(glm::ivec3 coords) : coords(coords), chunk(), needsRemeshing(false), mesh() {}
};

struct World {
    std::unordered_map<glm::ivec3, WorldChunk*> chunks;
    glm::ivec3 centerChunk;
    void updateRenderChunks();
    void init();
    void update(f32 time, f32 dt);
    void draw();
    void destroy();
};