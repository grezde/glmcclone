#include "entity.hpp"
#include "data.hpp"
#include "resources.hpp"
#include <cstring>

CuboidsEntityModel::Cuboid::Cuboid(DataEntry* de) {
    if(!de || !de->isMap()) return;
    DataEntry* uvde = de->schild("uv");
    if(!uvde || !uvde->isIVEC2()) return;
    this->uv = uvde->getIVEC2();
    DataEntry* dimsde = de->schild("dims");
    if(!dimsde || !dimsde->isIVEC3()) return;
    this->dimensions = dimsde->getIVEC3();
}

CuboidsEntityModel::AppliedCuboid::AppliedCuboid(const map<string, u32>& cuboidMap, DataEntry* de) {
    if(!de || !de->isMap()) return;

    DataEntry* facingde = de->schild("facing");
    if(!facingde || !facingde->isInteger()) facing = EAST;
    else facing = (Direction)(facingde->geti64()%DIRECTION_COUNT);

    DataEntry* downwardsde = de->schild("downwards");
    if(!downwardsde || !downwardsde->isInteger()) downwards = DOWN;
    else downwards = (Direction)(downwardsde->geti64()%DIRECTION_COUNT);

    DataEntry* flipde = de->schild("flip");
    if(!flipde || !flipde->isInteger()) flip = false;
    else flip = flipde->geti64();
    
    DataEntry* idde = de->schild("id");
    DataEntry* posde = de->schild("pos");
    if(!idde || !idde->isStringable() || !posde || !posde->isVEC3()) return;
    if(cuboidMap.find(idde->str) == cuboidMap.end()) return;
    id = cuboidMap.at(idde->str);
    pos = posde->getVEC3();

}

CuboidsEntityModel::Object::Object(const map<string, u32>& cuboidMap, const string& name, DataEntry* de) {
    this->name = name;
    if(!de || !de->isMap()) return;
    DataEntry* pivotde = de->schild("pivot");
    if(pivotde && pivotde->isVEC3()) this->pivot = pivotde->getVEC3();
    DataEntry* cuboidsDe = de->schild("cuboids");
    if(!cuboidsDe || !cuboidsDe->isListable()) return;
    for(DataEntry* apcubde : cuboidsDe->list) {
        this->cuboids.push_back(AppliedCuboid(cuboidMap, apcubde));
    }
}

void EntityModel::setTexture(DataEntry* de) {
    DataEntry* texture = de->schild("texture");
    if(!texture || !texture->isStringable() || !Registry::textures.has(texture->str))
        return;
    TextureRI& tri = Registry::textures[texture->str];
    if(tri.usage != TextureRI::GLTEXTURE)
        return;
    this->texture = tri.usageID;
}

CuboidsEntityModel::VertexIntermediary cuboidsVertices[DIRECTION_COUNT*4] = {
    { { 1, 0, 1 }, { 0, 0 } },
    { { 1, 0, 0 }, { 1, 0 } },
    { { 1, 1, 0 }, { 1, 1 } },
    { { 1, 1, 1 }, { 0, 1 } },
    
    { { 0, 0, 1 }, { 0, 0 } },
    { { 1, 0, 1 }, { 1, 0 } },
    { { 1, 1, 1 }, { 1, 1 } },
    { { 0, 1, 1 }, { 0, 1 } },

    { { 0, 0, 0 }, { 0, 0 } },
    { { 0, 0, 1 }, { 1, 0 } },
    { { 0, 1, 1 }, { 1, 1 } },
    { { 0, 1, 0 }, { 0, 1 } },

    { { 1, 0, 0 }, { 1, 0 } },
    { { 0, 0, 0 }, { 0, 0 } },
    { { 0, 1, 0 }, { 0, 1 } },
    { { 1, 1, 0 }, { 1, 1 } },

    { { 0, 1, 0 }, { 0, 0 } },
    { { 0, 1, 1 }, { 1, 0 } },
    { { 1, 1, 1 }, { 1, 1 } },
    { { 1, 1, 0 }, { 0, 1 } },

    { { 0, 0, 1 }, { 1, 0 } },
    { { 0, 0, 0 }, { 0, 0 } },
    { { 1, 0, 0 }, { 0, 1 } },
    { { 1, 0, 1 }, { 1, 1 } },
  
};

void CuboidsEntityModel::Cuboid::makeIntermediary(CuboidsEntityModel::VertexIntermediary* vi) {
    memcpy(vi, cuboidsVertices, 24*sizeof(VertexIntermediary));
    glm::vec3 sdimensions = glm::vec3(dimensions.z, dimensions.y, dimensions.x);
    for(u32 i=0; i<24; i++)
        vi[i].xyz = -0.5f*sdimensions + glm::vec3(vi[i].xyz) * sdimensions;
    glm::ivec2 multipliers[DIRECTION_COUNT] = { 
        { dimensions.x, -dimensions.y },
        { dimensions.z, -dimensions.y },
        { dimensions.x, -dimensions.y },
        { dimensions.z, -dimensions.y },
        { -dimensions.x, -dimensions.z },
        { -dimensions.x, dimensions.z },
    };
    glm::ivec2 offsets[DIRECTION_COUNT] = { 
        { 0, dimensions.y},
        { -dimensions.z, dimensions.y },
        { dimensions.x+dimensions.z, dimensions.y },
        { dimensions.x, dimensions.y },
        { dimensions.x, 0 },
        { 2*dimensions.x, -dimensions.z }
    };
    for(u32 i=0; i<24; i++)
        vi[i].uv = vi[i].uv * multipliers[i/4] + offsets[i/4] + uv;
}

void CuboidsEntityModel::AppliedCuboid::makeIntermediary(CuboidsEntityModel::VertexIntermediary* vi) {
    glm::mat3 rotation_mat = {};
    glm::ivec2 facingAS = directionToAxisAndSign[facing];
    glm::ivec2 downwardsAS = directionToAxisAndSign[downwards];
    if(downwardsAS.x == facingAS.x) return;
    glm::ivec2 other = { 0, 1 };
    if(downwardsAS.x == 0 || facingAS.x == 0)
        other.x = 1;
    if(downwardsAS.x == 1 || facingAS.x == 1)
        other.x = 2;
    other.y *= facingAS.y * downwardsAS.y;
    // due to default directions
    rotation_mat[facingAS.x][0] = facingAS.y;
    rotation_mat[downwardsAS.x][1] = -downwardsAS.y;
    rotation_mat[other.x][2] = other.y;
    bool isPositive = glm::determinant(rotation_mat) > 0;
    if(isPositive == flip)
        rotation_mat[other.x][2] = -other.y;
    for(u32 i=0; i<24; i++)
        vi[i].xyz = pos + rotation_mat * vi[i].xyz;
    if(flip) for(u32 dir=0; dir<6; dir++) {
        VertexIntermediary temp = vi[dir*4+0];
        vi[dir*4+0] = vi[dir*4+3];
        vi[dir*4+3] = temp;
        temp = vi[dir*4+1];
        vi[dir*4+1] = vi[dir*4+2];
        vi[dir*4+2] = temp;
    }
}

CuboidsEntityModel::CuboidsEntityModel(DataEntry* de) {
    if(!de || !de->isMap() || !de->has("cuboids") || !de->has("objects"))
        return;
    setTexture(de);
    DataEntry* cuboidsDE = de->schild("cuboids");
    DataEntry* objectsDE = de->schild("objects");
    map<string, u32> cuboidMap;
    if(!cuboidsDE->isMap() || !objectsDE->isMap()) return;
    for(const pair<string, DataEntry*> kc : cuboidsDE->dict) {
        cuboidMap[kc.first] = cuboids.size();
        cuboids.push_back(Cuboid(kc.second));
    }
    for(const pair<string, DataEntry*> kc : objectsDE->dict)
        objects.push_back(Object(cuboidMap, kc.first, kc.second));
}

void CuboidsEntityModel::makeMesh(Entity* entity) {
    SimpleMesh& mesh = entity->mesh;
    GLTexture& gltexture = Registry::glTextures.items[texture];
    mesh.indices.clear();
    mesh.vertices.clear();
    mesh.position = entity->pos;
    mesh.scaling = glm::vec3(1, 1, 1);

    for(Object& object : objects) for(AppliedCuboid& apc : object.cuboids) {
        VertexIntermediary vi[24];
        cuboids[apc.id].makeIntermediary(vi);
        apc.makeIntermediary(vi);
        
        for(u32 dir=0; dir<DIRECTION_COUNT; dir++) {
            for(u32 i=dir*4; i<(dir+1)*4; i++)
                mesh.vertices.push_back(SimpleVertex { 
                    .position = vi[i].xyz / (f32)Registry::ATLASTILE,
                    .color = glm::vec4(1.0, 1.0, 1.0, 1.0),
                    .texCoords = glm::vec2(vi[i].uv) / glm::vec2(gltexture.width, gltexture.height)
                });
            mesh.indices.push_back(mesh.vertices.size()-4 + 0);
            mesh.indices.push_back(mesh.vertices.size()-4 + 1);
            mesh.indices.push_back(mesh.vertices.size()-4 + 3);
            mesh.indices.push_back(mesh.vertices.size()-4 + 3);
            mesh.indices.push_back(mesh.vertices.size()-4 + 1);
            mesh.indices.push_back(mesh.vertices.size()-4 + 2);
        }
    }
    
    mesh.makeObjects();
}

EntityModel* CuboidsEntityModel::constructor(DataEntry *de) {
    return new CuboidsEntityModel(de); 
}

EntityType::EntityType(string entityName, DataEntry* de) : name(entityName) {
    this->aabb = {{0,0,0},{0,0,0}};
    this->model = nullptr;
    if(!de || !de->isMap()) return;

    DataEntry* model = de->schild("model");
    if(!model || !model->isStringable() || !Registry::entityModels.has(model->str)) return;
    this->model = Registry::entityModels[model->str].constructor(de);
    
    DataEntry* aabb = de->schild("aabb");
    if(!aabb || !aabb->isMap() || !aabb->has("start") || !aabb->has("dims")) return;
    DataEntry* start = aabb->schild("start"); 
    DataEntry* dims = aabb->schild("dims");
    if(!start->isVEC3() || !dims->isVEC3()) return;
    this->aabb = { start->getVEC3(), dims->getVEC3() };


}