#include "world.hpp"
#include "data.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include <cstdlib>
#include <cstring>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_int3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/norm.hpp>

bool Chunk::inBounds(ivec3 inChunkCoords) {
    return !(
        inChunkCoords.x < 0 || inChunkCoords.x >= (i32)CHUNKSIZE || 
        inChunkCoords.y < 0 || inChunkCoords.y >= (i32)CHUNKSIZE ||
        inChunkCoords.z < 0 || inChunkCoords.z >= (i32)CHUNKSIZE
    );
}

u32 Chunk::indexOf(ivec3 inChunkCoords) {
    return inChunkCoords.x + inChunkCoords.z*CHUNKSIZE + inChunkCoords.y*CHUNKSIZE*CHUNKSIZE;
}

vec3 harmonics[] = { {4.0f, 0.1f, 0.1f}, {2.0f, 0.2f, 0.2f}, {1.0f, 0.3f, 0.5f}, {1.0f, 0.5f, 0.3f} };
vec2 offsets[] = { {}, {}, {}, {} };

void Chunk::makeSin(ivec3 coords) {
    if(offsets[0].x == 0)
        for(u32 i=0; i<4; i++) {
            offsets[i].x = (f32)rand()/(f32)RAND_MAX;
            offsets[i].y = (f32)rand()/(f32)RAND_MAX;
        }
    for(u32 x=0; x<CHUNKSIZE; x++) for(u32 z=0; z<CHUNKSIZE; z++) {
        f32 heightf = 15.0f;
        for(u32 i=0; i<4; i++) {
            vec2 pos = vec2((f32)(coords.x*(i32)CHUNKSIZE+(i32)x), (f32)(coords.z*(i32)CHUNKSIZE+(i32)z));
            pos += offsets[i] * 2.0f * glm::pi<f32>();
            f32 v = harmonics[i].x * cosf(harmonics[i].y*pos.x) * cosf(harmonics[i].z*pos.y);
            heightf += v;
        }
        u32 height = heightf;
        for(u32 y=0; y<CHUNKSIZE; y++) {
            u32 i = indexOf({x, y, z});
            f32 r = (f32)rand()/(f32)RAND_MAX;
            if(y < height-4) {
                if(r < 0.1) blocks[i] = Registry::blocks.names["cobblestone"];
                else blocks[i] = Registry::blocks.names["stone"];
            } else if(y < height)
                blocks[i] = Registry::blocks.names["dirt"];
            else if(y == height) {
                if(r < 0.35) blocks[i] = Registry::blocks.names["sand"];
                //else if(r < 0.4) blocks[i] = Registry::blocks.names["red_sand"];
                else if(r < 0.45) blocks[i] = Registry::blocks.names["dirt"];
                else blocks[i] = Registry::blocks.names["grass"];
            }
            else 
                blocks[i] = 0;
            if(x == CHUNKSIZE/2 && z == CHUNKSIZE/2) {
                if(y > height && y < height+10)
                    blocks[i] = Registry::blocks.names["log/y"];
            }
        }
        
    }

}

void Chunk::makeRandom() {
    for(u32 x=0; x<CHUNKSIZE; x++)
    for(u32 z=0; z<CHUNKSIZE; z++)
    for(u32 y=0; y<CHUNKSIZE; y++) {
        u32 i = indexOf({x, y, z});
        f32 r = (f32)rand()/(f32)RAND_MAX;
        if(r > (f32)y/(f32)CHUNKSIZE)
            blocks[i] = rand() % (Registry::blocks.items.size()-1) + 1;
        else
            blocks[i] = 0;
    };
}

void Chunk::makeVoxelMesh(VoxelMesh& mesh, Chunk* neighbours[6]) const {

    BlockModel::DrawInfo di = { mesh, *this, { 
        neighbours[0], neighbours[1], neighbours[2], 
        neighbours[3], neighbours[4], neighbours[5]
    }, {0,0,0} };
    //mesh.vertices, mesh.indices;
    for(di.blockPos.x=0; di.blockPos.x<(i32)CHUNKSIZE; di.blockPos.x++)
    for(di.blockPos.z=0; di.blockPos.z<(i32)CHUNKSIZE; di.blockPos.z++)
    for(di.blockPos.y=0; di.blockPos.y<(i32)CHUNKSIZE; di.blockPos.y++) {
        u32 i = indexOf(di.blockPos);
        if(blocks[i] == 0)
            continue;
        BlockModel* model = Registry::blocks.items[blocks[i]]->model;
        if(!model)
            continue;
        model->addToMesh(di);

    };
}

void World::updateRenderChunks() {

}

void World::init() {
    i32 rd = 5;
    ivec3 start = {-rd, 0, -rd};
    ivec3 end = { rd, 1, rd};

    for(i32 x=start.x; x<end.x; x++) for(i32 z=start.z; z<end.z; z++) for(i32 y=start.y; y<end.y; y++) {
        ivec3 p = {x, y, z};
        
        WorldChunk* wc = new WorldChunk(p);
        //wc->chunk.makeRandom();
        wc->chunk.makeSin(p);
        wc->mesh.chunkCoords = p;
        chunks[p] = wc;
        
        UUID cown = UUID_make();
        Entity* cow = new Entity();
        cow->uuid = cown;
        cow->data = new DataEntry(DataEntry::MAP);
        cow->pos = vec3(p) * (f32)Chunk::CHUNKSIZE + vec3(18, 25, 18);
        cow->type = Registry::entities["cow"].id;
        cow->vel = {0,0,0};
        Registry::entities["cow"].model->makeMesh(cow);
        wc->entities[cown] = cow;
    }

    for(i32 x=start.x; x<end.x; x++) for(i32 z=start.z; z<end.z; z++) for(i32 y=start.y; y<end.y; y++) {
        ivec3 p = {x, y, z};
        WorldChunk* wc = chunks[p];
        Chunk* neighbours[6];
        for(u32 dir=0; dir < 6; dir++) {
            ivec3 np = p + directionVector[dir];
            if(chunks.find(np) != chunks.end())
                neighbours[dir] = &chunks[np]->chunk;
            else
                neighbours[dir] = nullptr;
        }
        wc->chunk.makeVoxelMesh(wc->mesh, neighbours);
        wc->mesh.makeObjects();
    }

    player = new Entity();
    player->type = Registry::entities.names.at("player");
    player->uuid = UUID_make();
    player->pos = { 21.0f, 26.0f, 21.0f };
    player->lookingAt = { -3.141f*0.75f, 0, 0 };
    player->vel = {0,0,0};
    player->acc = {0,0,0};
    chunks[chunkCoords(player->pos)]->entities[player->uuid] = player;

}

void World::draw(f32 time) const {

    camera.pos = player->pos + 1.75f;
    camera.angles = player->lookingAt;
    camera.makeMatrices();

    shader::bind(Registry::shaders["voxel"]);
    camera.setMatrices();
    gl::bindTexture(Registry::glTextures["atlas"].glid, 0);
    shader::setTexture("tex", 0);
    for(auto& p : chunks) {
        p.second->mesh.updateUniforms();
        p.second->mesh.draw();
    }

    shader::bind(Registry::shaders["simple"]);
    camera.setMatrices();
    shader::setTexture("tex", 0);
    for(auto& p : chunks) for(auto& pp : p.second->entities) {
        u32 textureID = Registry::entities.items[pp.second->type].model->texture;
        pp.second->updateMeshes(time);
        gl::bindTexture(Registry::glTextures.items[textureID].glid, 0);
        for(auto& mesh : pp.second->meshes) {
            mesh.updateUniforms();
            mesh.draw();
        }
    }
}

AABB playerBB = { {-0.4, -1.7, -0.4}, { 0.8, 1.95, 0.5 } };
bool inspectMode = false;

void World::update(f32 time, f32 dt) {
    (void) time;

    // adding forces
    for(pair<const ivec3, WorldChunk*>& chunkp : chunks) 
        for(pair<const UUID, Entity*>& entityp : chunkp.second->entities) {
            entityp.second->acc = {0,0,0};
            entityp.second->acc += vec3(0,-9.8,0);
        }

    static constexpr float cameraAngleSpeed = 2.0f;
    static constexpr float cameraSpeed = 8.0f;
    static constexpr float stoptime = 0.2f;
    
    vec3 movement = {0, 0, 0};
    if(Input::isPressed(GLFW_KEY_W))
        movement += vec3(0, 0, 1);
    if(Input::isPressed(GLFW_KEY_A))
        movement += vec3(-1, 0, 0);
    if(Input::isPressed(GLFW_KEY_S))
        movement += vec3(0, 0, -1);
    if(Input::isPressed(GLFW_KEY_D))
        movement += vec3(1, 0, 0);
    if(Input::isPressed(GLFW_KEY_UP))
        movement += vec3(0, 1, 0);
    if(Input::isPressed(GLFW_KEY_DOWN))
        movement += vec3(0, -1, 0);
    bool escape = Input::isPressed(GLFW_KEY_ESCAPE);
    if(Input::events["escape"].happened())
        Input::toggleCursor();
    if(Input::events["spectator_toggle"].happened())
        inspectMode = !inspectMode;
    if(inspectMode)
        player->acc = {};

    player->acc -= player->vel*stoptime*cameraSpeed;
    vec3 movementReal = movement.z * vec3(std::cos(player->lookingAt.x), 0, std::sin(player->lookingAt.x))
            + movement.y * vec3(0, 1, 0)
            + movement.x * vec3(-std::sin(player->lookingAt.x), 0, std::cos(player->lookingAt.x));
    player->acc += movementReal * cameraSpeed;

    if(Input::disabledCursor) {
        vec2 mc = Input::mouseDiff;
        mc.x /= window::width;
        mc.y /= window::height;
        player->lookingAt += vec3(mc.x, mc.y, 0) * cameraAngleSpeed;
        constexpr float PI = 3.14159268f;
        if(player->lookingAt.y > 0.5f*PI*0.99f)
            player->lookingAt.y = 0.5f*PI*0.99f;
        if(player->lookingAt.y < -0.5f*PI*0.99f)
            player->lookingAt.y = -0.5f*PI*0.99f;
        if(player->lookingAt.x > PI)
            player->lookingAt.x -= 2.0*PI;
        if(player->lookingAt.x < -PI)
            player->lookingAt.x += 2.0*PI;
    }

    for(pair<const ivec3, WorldChunk*>& chunkp : chunks) 
        for(pair<const UUID, Entity*>& entityp : chunkp.second->entities) {
            entityp.second->vel += entityp.second->acc*dt;
            vec3 newpos = entityp.second->pos + entityp.second->vel*dt;
            if(entityp.second == player && inspectMode) {
                player->pos = newpos;
                continue;
            }
            pair<vec3, vec3> result = collide(Registry::entities.items[entityp.second->type].aabb, entityp.second->pos, newpos);
            entityp.second->pos = result.first;
            entityp.second->vel -= glm::dot(entityp.second->vel, result.second) * result.second;
        }

    for(pair<const ivec3, WorldChunk*>& chunkp : chunks) {
        WorldChunk& chunk = *(chunkp.second);
        for(std::unordered_map<UUID, Entity*>::iterator it = chunk.entities.begin(); it != chunk.entities.end(); ) {
            Entity* entity = it->second;
            ivec3 chunkCoord = chunkCoords(entity->pos);
            if(chunk.coords != chunkCoord && chunks.find(chunkCoord) != chunks.end()) {
                chunk.entities.erase(it++);
                chunks[chunkCoord]->entities[entity->uuid] = entity;
            }
            else ++it;
        }
    }
    
}

inline ivec3 World::floor(vec3 coords) {
    return ivec3(glm::floor(coords));
}

inline ivec3 World::chunkCoords(vec3 coords) {
    return floor(coords / (f32)Chunk::CHUNKSIZE);
}

inline vec3 World::inChunkCoordsF(vec3 coords) {
    return coords - vec3(chunkCoords(coords))*(f32)Chunk::CHUNKSIZE;
}

inline ivec3 World::inChunkCoordsI(vec3 coords) {
    return floor(inChunkCoordsF(coords));
}

pair<vec3, vec3> World::collide(const AABB& aabb, vec3 oldpos, vec3 newpos) const {
    vec3 normal = {0,0,0};
    for(u32 i=0; i<8; i++) {
        vec3 dx = newpos + aabb.start;
        dx += vec3((i&1)*aabb.size.x, ((i&2)>>1)*aabb.size.y, ((i&4)>>2)*aabb.size.z);
        ivec3 dxi = ivec3(glm::floor(dx));
        ivec3 chunkCoord = chunkCoords(dxi);
        ivec3 blockCoords = inChunkCoordsI(dxi);
        if(chunks.find(chunkCoord) == chunks.end())
            continue;
        Chunk::blockID block = chunks.at(chunkCoord)->chunk.blocks[Chunk::indexOf(blockCoords)];
        if(block == 0)
            continue;
        return {oldpos, {0, 1, 0}};
    }
    return std::make_pair(newpos, vec3(0,0,0));
}

void World::destroy() {

    

}
