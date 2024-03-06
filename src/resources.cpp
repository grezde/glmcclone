#include "resources.hpp"
#include "base.hpp"
#include "data.hpp"
#include "renderer.hpp"
#include <cstring>

registry<Texture> Registry::textures;
registry<Block*> Registry::blocks;
registry<BlockModelConstructor> Registry::blockModels;
registry<u32> Registry::glTextures;
registry<u32> Registry::shaders;

Block::Block(string blockName, DataEntry* de) : name(blockName) {
    if(!de->isMap()) return;
    DataEntry* modelName = de->schild("model");
    if(modelName == nullptr || !modelName->isStringable() || !Registry::blockModels.has(modelName->str)) return;
    model = Registry::blockModels[modelName->str](de);
    DataEntry* solidityDE = de->schild("solidity");
    if(solidityDE == nullptr) solidity = 0b111111;
    else solidity = solidityDE->geti64(); // TODO: verify solidity
}

BlockModel* noModelConstructor(DataEntry* de) {
    return new NoModel();
}

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



void Registry::makeAtlas() {
    
}

void Registry::init() {

    // shaders
    vector<FileEntry> shadersFES = readFolderRecursively("assets/shaders");
    for(FileEntry& fe : shadersFES) {
        if(!fe.hasExtension("glsl"))
            continue;
        fe.removeExtension(4);
        if(!fe.hasExtension("vert"))
            continue;
        fe.removeExtension(4);
        string vertName = "assets/shaders/" + fe.name + ".vert.glsl";
        string fragName = "assets/shaders/" + fe.name + ".frag.glsl";
        if(!fileExists(fragName.c_str()))
            continue;
        u32 shader = gl::makeProgram(
            readFileString(vertName.c_str()), 
            readFileString(fragName.c_str())
        );
        shaders.add(fe.name, shader);
    }

    shaders.print(cout, "shaders");

    // textures
    vector<FileEntry> textureFES = readFolderRecursively("assets/textures");
    for(FileEntry fe : textureFES) {
        if(!fe.hasExtension("png"))
            continue;
        fe.removeExtension(3);
        textures.add(fe.name, { (u32)textures.items.size(), fe.name });
    }

    // block models
    blockModels.add("none", noModelConstructor);
    blockModels.add("cube", cubeModelConstructor);

    // blocks
    vector<FileEntry> blockFES = readFolder("assets/blocks");
    for(FileEntry fe : blockFES) {
        if(!fe.hasExtension("td"))
            continue;
        string filename = "assets/blocks/" + fe.name;
        DataEntry* de = DataEntry::readText(readFileString(filename.c_str()));
        fe.removeExtension(2);
        blocks.add(fe.name, new Block(fe.name, de));
        delete de;
    }

    // atlas
    makeAtlas();

}