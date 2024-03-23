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

    void makeSin(ivec3 coords);
    void makeRandom();

    static u32 indexOf(ivec3 inChunkCoords);
    static bool inBounds(ivec3 inChunkCoords);
    //void makeSimpleMesh(SimpleMesh& mesh);
    void makeVoxelMesh(VoxelMesh& mesh, Chunk* neighbours[6]) const;
};

struct WorldChunk {
    ivec3 coords;
    Chunk chunk;
    // TODO: consider moving this to the the world
    std::unordered_map<UUID, Entity*> entities;
    bool needsRemeshing;
    VoxelMesh mesh;

    WorldChunk(ivec3 coords) : coords(coords), chunk(), needsRemeshing(false), mesh() {}
};


struct World {
    std::unordered_map<ivec3, WorldChunk*> chunks;
    Entity* player;
    ivec3 centerChunk;
    void updateRenderChunks();
    void init();
    void update(f32 time, f32 dt);
    void draw(f32 time) const;
    void destroy();

    static ivec3 floor(vec3 coords);
    static ivec3 chunkCoords(vec3 coords);
    static vec3 inChunkCoordsF(vec3 coords);
    static ivec3 inChunkCoordsI(vec3 coords);
    // returns the new collided positon and the normal
    pair<vec3, vec3> collide(const AABB& aabb, vec3 oldpos, vec3 newpos) const;
};