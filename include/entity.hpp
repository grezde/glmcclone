#pragma once
#include "base.hpp"
#include "renderer.hpp"
#include "blocks.hpp"
#include <cstdlib>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_int3.hpp>
#include <glm/ext/vector_int4.hpp>

struct DataEntry;

struct AABB {
    glm::vec3 start;
    glm::vec3 size;
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