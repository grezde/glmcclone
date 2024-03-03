#pragma once
#include "base.hpp"
#include "data.hpp"
#include "game.hpp"

struct Block {
    u8 solidity; // to be checked later 
    u32 faces[6]; // textures for all the faces
    const char* name;
    Block(const char* blockName, DataEntry* de);
};

struct Registry {
    vector<Block> blocks;
    map<string, u32> blockNames;
    map<string, u32> textureNames;

    void init();
};