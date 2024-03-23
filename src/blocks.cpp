#include "blocks.hpp"
#include "data.hpp"
#include "resources.hpp"

const Direction directionOpposite[DIRECTION_COUNT] = {
    WEST, NORTH, EAST, SOUTH,
    DOWN, UP
};

ivec3 directionVector[DIRECTION_COUNT] = {
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

ivec2 directionToAxisAndSign[DIRECTION_COUNT] = {
    { 0, 1 },
    { 2, 1 },
    { 0, -1 }, 
    { 2, -1 },
    { 1, 1 },
    { 1, -1 }
};

Block::Block(string blockName, DataEntry* de) : name(blockName) {
    solidity = 0; model = nullptr;
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
    ivec3 xyz;
    ivec2 uv;
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
        ivec3 dv = directionVector[dir];
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
            ivec3 p = { (vertex.pos_ao&0x3F), (vertex.pos_ao&0xFC0)>>6,(vertex.pos_ao&0x3F000)>>12 };
            vec3 g = vec3(p) - vec3(drawinfo.blockPos);
            //cout << g.x << " " << g.y << " " << g.z << "\n";
            g = vec3(-1, -1, -1) + 2.0f*g;
            // g gives us one of the 8 corners, 1or -1 for each axis
            u32 neighbours = 0;
            vec3 ns[7] = { g, {g.x, g.y, 0}, {g.x, 0, g.z}, {0, g.y, g.z}, {g.x, 0, 0}, {0,g.y,0}, {0,0,g.z} };
            for(u32 nsi=0; nsi<7; nsi++) {
                ivec3 nsint = ns[nsi];
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