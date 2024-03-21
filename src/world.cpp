#include "world.hpp"
#include "data.hpp"
#include "game.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include <cstring>
#include <glm/ext/vector_int3.hpp>
#include <glm/matrix.hpp>

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

glm::ivec2 directionToAxisAndSign[DIRECTION_COUNT] = {
    { 0, 1 },
    { 2, 1 },
    { 0, -1 }, 
    { 2, -1 },
    { 1, 1 },
    { 1, -1 }
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

BlockModel* NoModel::constructor(DataEntry* de) {
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

BlockModel* CubeModel::constructor(DataEntry* de) {
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
            TextureRI texture = Registry::textures[p.second->str];
            for(u8 dir = 0; dir < 6; dir++) 
                if(set & (1 << dir))
                    model->faces[dir] = texture.usageID;
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

void Chunk::makeSin(glm::ivec3 coords) {
    for(u32 x=0; x<CHUNKSIZE; x++) for(u32 z=0; z<CHUNKSIZE; z++) {
        u32 height = 15 + 10*std::sin(0.1*(f32)(coords.x*CHUNKSIZE+x))*std::sin(0.1*(f32)(coords.z*CHUNKSIZE+z));
        for(u32 y=0; y<CHUNKSIZE; y++) {
            u32 i = indexOf({x, y, z});
            f32 r = (f32)rand()/(f32)RAND_MAX;
            if(y < height-4) {
                if(r < 0.1) blocks[i] = Registry::blocks.names["cobblestone"];
                else blocks[i] = Registry::blocks.names["stone"];
            } else if(y < height)
                blocks[i] = Registry::blocks.names["dirt"];
            else if(y == height) {
                if(r < 0.05) blocks[i] = Registry::blocks.names["dirt"];
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
    // 300 chunks got 25 fps with simple shader and pretty good laptop gpu
    // and a very long load time (probabily transfering from cpu to gpu)
    // Using reduced memory, I got it to 40 fps (at full battery)
    for(u32 x=0; x<5; x++) for(u32 z=0; z<5; z++) for(u32 y=0; y<1; y++) {
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
    shader::bind(Registry::shaders["voxel"]);
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
    gl::bindTexture(Registry::glTextures["entities/cow_png"].glid, 0);
    shader::setTexture("tex", 0);
    
    for(auto& p : chunks) for(auto& pp : p.second->entities) {
        pp.second->mesh.updateUniforms();
        pp.second->mesh.draw();
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
    { { 0, 1, 1 }, { 0, 1 } },
    { { 1, 1, 1 }, { 1, 1 } },
    { { 1, 1, 0 }, { 1, 0 } },

    { { 0, 0, 1 }, { 0, 1 } },
    { { 0, 0, 0 }, { 0, 0 } },
    { { 1, 0, 0 }, { 1, 0 } },
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
        { -dimensions.x, dimensions.z },
        { -dimensions.x, dimensions.z },
    };
    glm::ivec2 offsets[DIRECTION_COUNT] = { 
        { 0, dimensions.y},
        { -dimensions.z, dimensions.y },
        { dimensions.x+dimensions.z, dimensions.y },
        { dimensions.x, dimensions.y },
        { dimensions.x, -dimensions.z },
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
    glm::ivec2 other = { 0, flip ? -1 : 1 };
    if(downwardsAS.x == 0 || facingAS.x == 0)
        other.x = 1;
    if(downwardsAS.x == 1 || facingAS.x == 1)
        other.x = 2;
    other.y *= facingAS.y * downwardsAS.y;
    // due to default directions
    rotation_mat[facingAS.x][0] = facingAS.y;
    rotation_mat[downwardsAS.x][1] = -downwardsAS.y;
    rotation_mat[other.x][2] = other.y;
    if(glm::determinant(rotation_mat) < 0)
        rotation_mat[other.x][2] = -other.y;
    for(u32 i=0; i<24; i++)
        vi[i].xyz = pos + rotation_mat * vi[i].xyz;
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
                mesh.vertices.push_back((SimpleVertex) { 
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