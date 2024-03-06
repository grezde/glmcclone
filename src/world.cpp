#include "world.hpp"
#include "resources.hpp"

const Direction directionOpposite[DIRECTION_COUNT+1] = {
    WEST, NORTH, EAST, SOUTH,
    DOWN, UP, DIRECTION_COUNT
};

glm::ivec3 directionVector[DIRECTION_COUNT+1] = {
    { 1, 0, 0},
    { 0, 0, 1},
    {-1, 0, 0},
    { 0, 0,-1},
    { 0, 1, 0},
    { 0,-1, 0},
    { 0, 0, 0}
};


Block::Block(string blockName, DataEntry* de) : name(blockName) {
    if(!de || !de->isMap()) return;
    DataEntry* modelName = de->schild("model");
    if(modelName == nullptr || !modelName->isStringable() || !Registry::blockModels.has(modelName->str)) return;
    model = Registry::blockModels[modelName->str](de);
    DataEntry* solidityDE = de->schild("solidity");
    if(solidityDE == nullptr) solidity = 0b111111;
    else solidity = solidityDE->geti64(); // TODO: verify solidity
}

BlockModel* noModelConstructor(DataEntry* de) {
    (void) de;
    return new NoModel();
}

SimpleVertex cubeFaces[DIRECTION_COUNT*4] = {
    { { 1, 0, 0 }, {1,1,1}, { 1, 0 } },
    { { 1, 1, 0 }, {1,1,1}, { 1, 1 } },
    { { 1, 1, 1 }, {1,1,1}, { 0, 1 } },
    { { 1, 0, 1 }, {1,1,1}, { 0, 0 } },

    { { 0, 0, 1 }, {1,1,1}, { 0, 0 } },
    { { 1, 0, 1 }, {1,1,1}, { 1, 0 } },
    { { 1, 1, 1 }, {1,1,1}, { 1, 1 } },
    { { 0, 1, 1 }, {1,1,1}, { 0, 1 } },

    { { 0, 0, 0 }, {1,1,1}, { 0, 0 } },
    { { 0, 0, 1 }, {1,1,1}, { 1, 0 } },
    { { 0, 1, 1 }, {1,1,1}, { 1, 1 } },
    { { 0, 1, 0 }, {1,1,1}, { 0, 1 } },

    { { 0, 0, 0 }, {1,1,1}, { 0, 0 } },
    { { 0, 1, 0 }, {1,1,1}, { 0, 1 } },
    { { 1, 1, 0 }, {1,1,1}, { 1, 1 } },
    { { 1, 0, 0 }, {1,1,1}, { 1, 0 } },

    { { 0, 1, 0 }, {1,1,1}, { 0, 0 } },
    { { 0, 1, 1 }, {1,1,1}, { 0, 1 } },
    { { 1, 1, 1 }, {1,1,1}, { 1, 1 } },
    { { 1, 1, 0 }, {1,1,1}, { 1, 0 } },

    { { 0, 0, 0 }, {1,1,1}, { 0, 0 } },
    { { 1, 0, 0 }, {1,1,1}, { 1, 0 } },
    { { 1, 0, 1 }, {1,1,1}, { 1, 1 } },
    { { 0, 0, 1 }, {1,1,1}, { 0, 1 } },
};

BlockModel* cubeModelConstructor(DataEntry* de) {
    CubeModel* model = new CubeModel();
    DataEntry* texturesBlock = de->schild("textures");
    if(!texturesBlock || !texturesBlock->isMap()) 
        return model;
    for(const std::pair<string, DataEntry*> p : texturesBlock->dict) {
        u8 set = 0b000000;
        if(!p.second->isStringable())
            continue;
        if(p.first == "all")        set = 0b111111;
        else if(p.first == "east")  set = 0b000001;
        else if(p.first == "south") set = 0b000010;
        else if(p.first == "west")  set = 0b000100;
        else if(p.first == "north") set = 0b001000;
        else if(p.first == "up")    set = 0b010000;
        else if(p.first == "down")  set = 0b100000;
        else if(p.first == "y")     set = 0b110000;
        else if(p.first == "tops")  set = 0b110000;
        else if(p.first == "x")     set = 0b000101;
        else if(p.first == "z")     set = 0b001010;
        else if(p.first == "sides") set = 0b001111;
        if(!Registry::textures.has(p.second->str))
            continue;
        Texture texture = Registry::textures[p.second->str];
        for(u8 dir = 0; dir < 6; dir++)
            if(set & (1 << dir)) {
                model->faces[dir] = texture.id;
            }
    }
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
        glm::ivec3 blockToCheck = drawinfo.blockPos + dv;
        if(drawinfo.chunk.inBounds(blockToCheck)) {
            // TODO: implement solidity
            if(drawinfo.chunk.blocks[drawinfo.chunk.indexOf(blockToCheck)] != 0)
                continue;
        }
        //; //draw face 
        //glm::vec3 facepos = glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f) + glm::vec3(dv)*0.5f;
        u8 textureID = faces[dir];
        for(u32 faceVertex=0; faceVertex<4; faceVertex++) {
            SimpleVertex vertex = cubeFaces[dir*4+faceVertex];
            vertex.texCoords = glm::vec2(textureID%16, 15-textureID/16)/16.0f + vertex.texCoords/16.0f;
            vertex.position += drawinfo.blockPos;
            drawinfo.mesh.vertices.push_back(vertex);
        }

        // Ambient oclusion demo
        // TODO: the ambient oclusion sucks
        // for each vertex we check the block on the sides on color it less the more neighbours it has
        for(u32 faceVertex=0; faceVertex<4; faceVertex++) {
            SimpleVertex& vertex = drawinfo.mesh.vertices[drawinfo.mesh.vertices.size()-4+faceVertex];
            glm::vec3 g = vertex.position - glm::vec3(drawinfo.blockPos);
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
            vertex.color *= 1.0f - neighbours * 0.07f;
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

void Chunk::makeSimpleMesh(SimpleMesh& mesh) {

    BlockModel::DrawInfo di = { mesh, *this, {0,0,0} };
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

void World::init() {
    chunks.push_back(WorldChunk { 
        .coords = { 0,0,0 }, 
        .chunk = Chunk(), 
        .mesh = SimpleMesh() 
    });
    chunks[0].chunk.makeRandom();
    chunks[0].chunk.makeSimpleMesh(chunks[0].mesh);
    chunks[0].mesh.makeObjects();
}

void World::draw() {
    chunks[0].mesh.updateUniforms();
    chunks[0].mesh.draw();
}