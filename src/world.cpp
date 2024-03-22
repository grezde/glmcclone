#include "world.hpp"
#include "data.hpp"
#include "game.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include <cstring>
#include <glm/ext/vector_int3.hpp>
#include <glm/matrix.hpp>

bool Chunk::inBounds(glm::ivec3 inChunkCoords) {
    return !(
        inChunkCoords.x < 0 || inChunkCoords.x >= (i32)CHUNKSIZE || 
        inChunkCoords.y < 0 || inChunkCoords.y >= (i32)CHUNKSIZE ||
        inChunkCoords.z < 0 || inChunkCoords.z >= (i32)CHUNKSIZE
    );
}

u32 Chunk::indexOf(glm::ivec3 inChunkCoords) {
    return inChunkCoords.x + inChunkCoords.z*CHUNKSIZE + inChunkCoords.y*CHUNKSIZE*CHUNKSIZE;
}

void Chunk::makeSin(glm::ivec3 coords) {
    for(u32 x=0; x<CHUNKSIZE; x++) for(u32 z=0; z<CHUNKSIZE; z++) {
        u32 height = 15.0f + 10.0f*std::sin(0.1*(f32)(coords.x*CHUNKSIZE+x))*std::sin(0.1*(f32)(coords.z*CHUNKSIZE+z));
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
    glm::ivec3 start = {-rd, 0, -rd};
    glm::ivec3 end = { rd, 1, rd};

    for(i32 x=start.x; x<end.x; x++) for(i32 z=start.z; z<end.z; z++) for(i32 y=start.y; y<end.y; y++) {
        glm::ivec3 p = {x, y, z};
        
        WorldChunk* wc = new WorldChunk(p);
        //wc->chunk.makeRandom();
        wc->chunk.makeSin(p);
        wc->mesh.chunkCoords = p;
        chunks[p] = wc;
        
        UUID cown = UUID_make();
        Entity* cow = new Entity();
        cow->uuid = cown;
        cow->data = new DataEntry(DataEntry::MAP);
        cow->pos = glm::vec3(p) * (f32)Chunk::CHUNKSIZE + glm::vec3(18, 25, 18);
        cow->type = Registry::entities["cow"].id;
        cow->vel = {0,0,0};
        Registry::entities["cow"].model->makeMesh(cow);
        wc->entities[cown] = cow;
    }

    for(i32 x=start.x; x<end.x; x++) for(i32 z=start.z; z<end.z; z++) for(i32 y=start.y; y<end.y; y++) {
        glm::ivec3 p = {x, y, z};
        WorldChunk* wc = chunks[p];
        Chunk* neighbours[6];
        for(u32 dir=0; dir < 6; dir++) {
            glm::ivec3 np = p + directionVector[dir];
            if(chunks.find(np) != chunks.end())
                neighbours[dir] = &chunks[np]->chunk;
            else
                neighbours[dir] = nullptr;
        }
        wc->chunk.makeVoxelMesh(wc->mesh, neighbours);
        wc->mesh.makeObjects();
    }
}

void World::draw(f32 time) {

    glm::vec3& cameraPos = Game::cameraPos;
    glm::vec2& cameraAngle = Game::cameraAngle;
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + glm::vec3(
        std::cos(cameraAngle.x)*std::cos(cameraAngle.y),
        std::sin(cameraAngle.y),
        std::sin(cameraAngle.x)*std::cos(cameraAngle.y)
    ), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(70.0f), (f32)window::width/(f32)window::height, 0.01f, 1000.0f);
    
    shader::bind(Registry::shaders["voxel"]);
    shader::setMat4("view", view);
    shader::setMat4("proj", proj);
    gl::bindTexture(Registry::glTextures["atlas"].glid, 0);
    shader::setTexture("tex", 0);
    for(auto& p : chunks) {
        p.second->mesh.updateUniforms();
        p.second->mesh.draw();
    }

    shader::bind(Registry::shaders["simple"]);
    shader::setMat4("view", view);
    shader::setMat4("proj", proj);
    shader::setMat4("submodel", glm::mat4(1));
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

void World::update(f32 time, f32 dt) {
    (void) time;

    if(Game::inspectMode) {
        Game::cameraVel = {0,0,0};
        return;
    }
    Game::cameraVel += glm::vec3(0, -9.8, 0) * dt;
    glm::vec3 newCameraPos = Game::cameraPos + Game::cameraVel * dt;
    for(u32 i=0; i<8; i++) {
        glm::vec3 dx = playerBB.start;
        dx += glm::vec3((i&1)*playerBB.size.x, ((i&2)>>1)*playerBB.size.y, ((i&4)>>2)*playerBB.size.z);
        glm::ivec3 dxi = newCameraPos + dx;
        glm::ivec3 chunkCoords = dxi / (i32)Chunk::CHUNKSIZE;
        glm::ivec3 blockCoords = dxi - chunkCoords*(i32)Chunk::CHUNKSIZE;
        if(chunks.find(chunkCoords) == chunks.end())
            continue;
        Chunk::blockID block = chunks[chunkCoords]->chunk.blocks[Chunk::indexOf(blockCoords)];
        if(block == 0)
            continue;
        else {
            Game::cameraVel = {0,0,0};
            return;
        }
    }
    Game::cameraPos = newCameraPos;
}

void World::destroy() {

}
