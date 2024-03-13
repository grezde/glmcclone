#include "world.hpp"
#include "game.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include <cstring>
#include <glm/ext/vector_int3.hpp>

const Direction directionOpposite[DIRECTION_COUNT] = {
    WEST, NORTH, EAST, SOUTH,
    DOWN, UP
};

glm::ivec3 directionVector[DIRECTION_COUNT] = {
    { 1, 0, 0},
    { 0, 0, 1},
    {-1, 0, 0},
    { 0, 0,-1},
    { 0, 1, 0},
    { 0,-1, 0}
};

const char* directionNames[DIRECTION_COUNT] = {
    "east", "south", "west", "north", "up", "down"
};

Block::Block(string blockName, DataEntry* de) : name(blockName) {
    solidity = 0;
    if(!de || !de->isMap()) return;
    DataEntry* modelName = de->schild("model");
    if(modelName == nullptr || !modelName->isStringable() || !Registry::blockModels.has(modelName->str)) return;
    model = Registry::blockModels[modelName->str].constructor(de);
    DataEntry* solidityDE = de->schild("solidity");
    if(solidityDE == nullptr) 
        solidity = Registry::blockModels[modelName->str].defaultSolidity;
    else if(solidityDE->isInteger())
        solidity = solidityDE->geti64(); 
    // TODO: add error message to block creation
}

BlockModel* noModelConstructor(DataEntry* de) {
    (void) de;
    return new NoModel();
}

// TODO: remove u and v because they are populated on the spot
struct VoxelVertexIntermediary {
    glm::ivec3 xyz;
    glm::ivec2 uv;
};

VoxelVertexIntermediary cubeFacesVV[DIRECTION_COUNT*4] = {
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
    { { 0, 1, 1 }, { 0, 1 } },
    { { 1, 1, 1 }, { 1, 1 } },
    { { 1, 1, 0 }, { 1, 0 } },

    { { 0, 0, 1 }, { 0, 1 } },
    { { 0, 0, 0 }, { 0, 0 } },
    { { 1, 0, 0 }, { 1, 0 } },
    { { 1, 0, 1 }, { 1, 1 } },
  
};

u8 CubeModel::getPermutation(Direction facedir, Direction texdir, bool flip) {
    //static const u8 zerozero[DIRECTION_COUNT] = { 3, 0, 0, 3, 0, 1 };

    u8 rot = 0; // number from 0 to 4 representing clockwise rotation of the texture 
    #define dcase(a, b, c) case a*DIRECTION_COUNT+b: rot = c; break;
    // maybe there is a formula for this but too lazy
    switch(facedir*DIRECTION_COUNT+texdir) {
        dcase(EAST, UP, 0)
        dcase(EAST, NORTH, 1)
        dcase(EAST, DOWN, 2)
        dcase(EAST, SOUTH, 3)
        dcase(SOUTH, UP, 0)
        dcase(SOUTH, EAST, 1)
        dcase(SOUTH, DOWN, 2)
        dcase(SOUTH, WEST, 3)
        dcase(WEST, UP, 0)
        dcase(WEST, SOUTH, 1)
        dcase(WEST, DOWN, 2)
        dcase(WEST, NORTH, 3)
        dcase(NORTH, UP, 0)
        dcase(NORTH, WEST, 1)
        dcase(NORTH, DOWN, 2)
        dcase(NORTH, EAST, 3)
        dcase(UP, EAST, 0)
        dcase(UP, SOUTH, 1)
        dcase(UP, WEST, 2)
        dcase(UP, NORTH, 3)
        dcase(DOWN, WEST, 0)
        dcase(DOWN, NORTH, 1)
        dcase(DOWN, EAST, 2)
        dcase(DOWN, SOUTH, 3)
    }
    #undef dcase
    u8 result = (4-rot)%4;
    if(flip) result = 4 + 4-result;
    return result;
}

BlockModel* cubeModelConstructor(DataEntry* de) {
    CubeModel* model = new CubeModel();
    DataEntry* texturesBlock = de->schild("textures");
    if(!texturesBlock || !texturesBlock->isMap()) 
        return model;
    bool flips[DIRECTION_COUNT] = {};
    Direction dirs[DIRECTION_COUNT] = {UP,UP,UP,UP,EAST,WEST};
    for(const std::pair<string, DataEntry*> p : texturesBlock->dict) {
        if(p.first.size() == 0)
            continue;
        u8 sit = 0;
        u32 stroff = 0;
        if(p.first.starts_with("orient_")) {
            stroff = 7;
            sit = 1;
        }
        else if(p.first.starts_with("flip_")) {
            stroff = 5;
            sit = 2;
        }
        u8 set = 0b000000;
        #define cmp(x, y) if(p.first.compare(stroff, p.first.size()-stroff+1, x) == 0) set = y
            cmp("all", 0b111111);
            else cmp("east", 0b000001);
            else cmp("south", 0b000010);
            else cmp("west", 0b000100);
            else cmp("north", 0b001000);
            else cmp("up", 0b010000);
            else cmp("down", 0b100000);
            else cmp("y", 0b110000);
            else cmp("tops", 0b110000);
            else cmp("x", 0b000101);
            else cmp("z", 0b001010);
            else cmp("sides", 0b001111);
        #undef cmp
        if(sit == 0) {
            if(!p.second->isStringable() || !Registry::textures.has(p.second->str))
                continue;
            Texture texture = Registry::textures[p.second->str];
            for(u8 dir = 0; dir < 6; dir++) 
                if(set & (1 << dir))
                    model->faces[dir] = texture.atlasID;
        }
        else if(sit == 1) {
            if(!p.second->isInteger() || (u8)p.second->geti64() >= DIRECTION_COUNT)
                continue;
            Direction facedir = (Direction)p.second->geti64();
            for(u8 dir = 0; dir < 6; dir++) 
                if(set & (1 << dir)) {
                    if(facedir == dir || facedir == directionOpposite[dir])
                        continue;
                    dirs[dir] = facedir;
                }
        }
        else if(sit == 2) {
            if(!p.second->isInteger())
                continue;
            for(u8 dir = 0; dir < 6; dir++) 
                if(set & (1 << dir))
                    flips[dir] = p.second->geti64();
        }
    }
    for(u8 dir = 0; dir < 6; dir++)
        model->permutions[dir] = CubeModel::getPermutation((Direction)dir, dirs[dir], flips[dir]);
    return model;
}

void CubeModel::addToMesh(BlockModel::DrawInfo drawinfo) {
    // pseudo code:
    // for direction //
        // check if there is a block in this chunk in that direction
        // if not, render that face
        // get solidity of block in that direction
        // if solidity is 0, render that face
        // otherwise skip

    for(u32 dir=0; dir<DIRECTION_COUNT; dir++) {
        glm::ivec3 dv = directionVector[dir];
        Chunk::blockID bid = 0;
        if(drawinfo.chunk.inBounds(drawinfo.blockPos + dv))
            bid = drawinfo.chunk.blocks[Chunk::indexOf(drawinfo.blockPos + dv)];
        else if(drawinfo.neighbours[dir] != nullptr)
            bid = drawinfo.neighbours[dir]->blocks[Chunk::indexOf(drawinfo.blockPos + dv - dv*(i32)Chunk::CHUNKSIZE)];
        else goto draw;
        if((Registry::blocks.items[bid]->solidity & (1<<dir)) == 0)
            goto draw;
        continue;

        draw:
        // texture orientation
        u8 textureID = faces[dir];
        VoxelVertexIntermediary vvi[4];
        for(u32 fv=0; fv<4; fv++)
            vvi[fv] = cubeFacesVV[dir*4+fv];
        u32 zerozero = permutions[dir] & 0b011;
        bool flip = permutions[dir] & 0b100;
        vvi[zerozero].uv = {0,0};
        vvi[(zerozero+2)%4].uv = {1,1};
        vvi[flip ? (zerozero+3)%4 : (zerozero+1)%4].uv = {1, 0};
        vvi[!flip ? (zerozero+3)%4 : (zerozero+1)%4].uv = {0, 1};
        
        // compression for shader
        for(u32 fv=0; fv<4; fv++) {
            vvi[fv].uv.x += textureID % Registry::ATLASDIM;
            vvi[fv].uv.y += Registry::ATLASDIM-1 - textureID/Registry::ATLASDIM;
            vvi[fv].xyz += drawinfo.blockPos;
            VoxelVertex vv;
            vv.pos_ao    = (vvi[fv].xyz.x) | (vvi[fv].xyz.y << 6) | (vvi[fv].xyz.z << 12);
            vv.texCoords = (vvi[fv].uv.x) | (vvi[fv].uv.y << 8);
            drawinfo.mesh.vertices.push_back(vv);
        }

        // Ambient oclusion demo
        // TODO: the ambient oclusion sucks
        // for each vertex we check the block on the sides on color it less the more neighbours it has
        for(u32 faceVertex=0; faceVertex<4; faceVertex++) {
            VoxelVertex& vertex = drawinfo.mesh.vertices[drawinfo.mesh.vertices.size()-4+faceVertex];
            glm::ivec3 p = { (vertex.pos_ao&0x3F), (vertex.pos_ao&0xFC0)>>6,(vertex.pos_ao&0x3F000)>>12 };
            glm::vec3 g = glm::vec3(p) - glm::vec3(drawinfo.blockPos);
            //cout << g.x << " " << g.y << " " << g.z << "\n";
            g = glm::vec3(-1, -1, -1) + 2.0f*g;
            // g gives us one of the 8 corners, 1or -1 for each axis
            u32 neighbours = 0;
            glm::vec3 ns[7] = { g, {g.x, g.y, 0}, {g.x, 0, g.z}, {0, g.y, g.z}, {g.x, 0, 0}, {0,g.y,0}, {0,0,g.z} };
            for(u32 nsi=0; nsi<7; nsi++) {
                glm::ivec3 nsint = ns[nsi];
                //cout << "{" << nsint.x << " " << nsint.y << " " << nsint.z << "} ";
                nsint += drawinfo.blockPos;
                if(drawinfo.chunk.inBounds(nsint)) {
                    if(drawinfo.chunk.blocks[drawinfo.chunk.indexOf(nsint)] != 0)
                        neighbours++;
                }
            }
            vertex.pos_ao |= (neighbours << 18);
            //cout << vertex.x << " " << vertex.y << " " << vertex.z << " -> ";
            //vertex.pos_ao = neighbours*64*64*64 + vertex.pos_ao;
            //cout << (vertex.pos_ao % 64) << " " <<
            //    ((vertex.pos_ao/64) % 64) << " " <<
            //    ((vertex.pos_ao/64/64) % 64) << " " <<
            //    ((vertex.pos_ao/64/64/64) % 8) << "\n";
        }

        drawinfo.mesh.indices.push_back(drawinfo.mesh.vertices.size()-4 + 0);
        drawinfo.mesh.indices.push_back(drawinfo.mesh.vertices.size()-4 + 1);
        drawinfo.mesh.indices.push_back(drawinfo.mesh.vertices.size()-4 + 3);
        drawinfo.mesh.indices.push_back(drawinfo.mesh.vertices.size()-4 + 3);
        drawinfo.mesh.indices.push_back(drawinfo.mesh.vertices.size()-4 + 1);
        drawinfo.mesh.indices.push_back(drawinfo.mesh.vertices.size()-4 + 2);
    }
}


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
    // 300 chunks got 25 fps with simple shader and pretty good laptop gpu
    // and a very long load time (probabily transfering from cpu to gpu)
    // Using reduced memory, I got it to 40 fps (at full battery)
    for(u32 x=0; x<5; x++) for(u32 z=0; z<5; z++) for(u32 y=0; y<1; y++) {
        glm::ivec3 p = {x, y, z};
        
        WorldChunk* wc = new WorldChunk { 
            .coords = p, 
            .chunk = Chunk(), 
            .mesh = VoxelMesh() 
        };
        wc->chunk.makeRandom();
        wc->mesh.chunkCoords = p;
        chunks[p] = wc;
    }

    for(u32 x=0; x<5; x++) for(u32 z=0; z<5; z++) for(u32 y=0; y<1; y++) {
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

void World::draw() {
    for(auto& p : chunks) {
        p.second->mesh.updateUniforms();
        p.second->mesh.draw();
    }
}

AABB playerBB = { {-0.4, -1.7, -0.4}, { 0.8, 1.95, 0.5 } };

void World::update(f32 time, f32 dt) {
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