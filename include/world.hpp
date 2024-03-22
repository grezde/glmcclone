#pragma once
#include "data.hpp"
#include "entity.hpp"
#include "renderer.hpp"
#include <cstdlib>
#include <glm/ext/vector_int3.hpp>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

struct VoxelMesh;

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