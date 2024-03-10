#include "resources.hpp"
#include "base.hpp"
#include "data.hpp"
#include "renderer.hpp"
#include <cstring>
#include <stb_image.h>
#include <stb_image_write.h>

registry<Texture> Registry::textures;
registry<Block*> Registry::blocks;
registry<BlockModelConstructor> Registry::blockModels;
registry<u32> Registry::glTextures;
registry<u32> Registry::shaders;
registry<vector<string>> Registry::enums;

u32* Registry::makeAtlas() {
    u32* data = (u32*)malloc(4*ATLASTILE*ATLASTILE*ATLASDIM*ATLASDIM);
    memset(data, 0, 4*ATLASTILE*ATLASTILE*ATLASDIM*ATLASDIM);
    for(u32 y=0; y<ATLASTILE; y++) for(u32 x=0; x<ATLASTILE; x++) {
        u32 mag = (x < 8) ^ (y < 8);
        data[y*ATLASTILE*ATLASDIM+x] = mag ? 0xFFFF00FF : 0xFF000000;
    }
    return data;
}

void Registry::addToAtlas(u32 *atlasData, u32 &index, const string& folder) {
    for(auto& p : textures.names) {
        if(p.first.compare(0, folder.size(), folder) != 0)
            continue;
        if(p.first.size() < folder.size()+2)
            continue;
        if(index == ATLASTILE*ATLASTILE)
            ERR_EXIT("Atlas too small");
        string filename = "assets/textures/" + p.first + ".png";
        i32 imageWidth, imageHeight, imageChannels;
        u8* newdata = stbi_load(filename.c_str(), &imageWidth, &imageHeight, &imageChannels, 4);
        // cout << "ATLAS " << p.first << " at " << filename << ": " << imageChannels << " " << imageWidth << " " << imageHeight << "\n";
        if(newdata == nullptr) ERR_EXIT("Could not load image " << p.first << ", " << filename);
        if(imageWidth != ATLASTILE || imageHeight != ATLASTILE)
            ERR_EXIT("Image not in apropriate format " << p.first);
        if(imageChannels == 3)
            imageChannels = 4;
        if(imageChannels != 4)
            ERR_EXIT("Image not in apropriate format " << p.first);
        for(u32 y=0; y < ATLASTILE; y++) for(u32 x=0; x < ATLASTILE; x++)
            atlasData[((index/ATLASDIM)*ATLASTILE + y)*ATLASTILE*ATLASDIM + (index%ATLASDIM)*ATLASTILE + x] = ((u32*)newdata)[ATLASTILE*y+x];
        stbi_image_free(newdata);
        textures.items[p.second].atlasID = index;
        index++;
    }
}

u32 Registry::finishAtlas(u32* atlasData) {
    #ifdef DEBUG
        stbi_write_png("output/atlas_new.png", ATLASTILE*ATLASDIM, ATLASTILE*ATLASDIM, 4, atlasData, ATLASTILE*ATLASDIM*4);
    #endif
    // Finally we must flip everything in acordance to OpenGL
    for(u32 y=0; y<ATLASTILE*ATLASDIM/2; y++) for(u32 x=0; x<ATLASTILE*ATLASDIM; x++) {
        u32 temp = atlasData[y*ATLASTILE*ATLASDIM+x];
        atlasData[y*ATLASTILE*ATLASDIM+x] = atlasData[(ATLASDIM*ATLASTILE-y-1)*ATLASTILE*ATLASDIM+x];
        atlasData[(ATLASDIM*ATLASTILE-y-1)*ATLASTILE*ATLASDIM+x] = temp;
    };
    u32 t = gl::textureFromMemory((u8*)atlasData, ATLASDIM*ATLASTILE, ATLASDIM*ATLASTILE, 4);
    free(atlasData);
    return t;
}

void addBlockToRegistry(DataEntry* de, const string& name) {
    if(!de || !de->isMap()) return;
    if(!de->has("variants")) {
        Registry::blocks.add(name, new Block(name, de));
        return;
    }
    DataEntry* variants = de->child("variants");
    de->dict.erase(de->dict.find("variants"));
    if(!variants->isMap()) return;
    for(const std::pair<string, DataEntry*> p : variants->dict) {
        string variantName = name + "/" + p.first;
        DataEntry* variantDE = de->copy();
        variantDE->mergeStructure(p.second);
        Registry::blocks.add(variantName, new Block(variantName, variantDE));
        delete variantDE;
    }
    de->dict["variants"] = variants;
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

    // textures
    vector<FileEntry> textureFES = readFolderRecursively("assets/textures");
    textures.add("missing", { 0, 0, "missing" });
    for(FileEntry fe : textureFES) {
        if(!fe.hasExtension("png"))
            continue;
        fe.removeExtension(3);
        textures.add(fe.name, { (u32)textures.items.size(), 0, fe.name });
    }

    // block models
    blockModels.add("none", noModelConstructor);
    blockModels.add("cube", cubeModelConstructor);

    // atlas
    u32* atlasData = makeAtlas();
    u32 atlasIndex = 1;
    addToAtlas(atlasData, atlasIndex, "blocks");
    glTextures.add("atlas", finishAtlas(atlasData));

    // enums 
    DataEntry* enumsDE = DataEntry::readText(readFileString("assets/enums.td"));
    if(enumsDE->isMap()) {
        // TODO: what is this warning
        for(const std::pair<string, DataEntry*>& p : enumsDE->dict) {
            if(!p.second->isListable())
                continue;
            vector<string> vs;
            for(const DataEntry* item : p.second->list) {
                if(!item->isStringable())
                    continue;
                vs.push_back(item->str);
            }
            enums.add(p.first, vs);
        }
    }
    delete enumsDE;

    // blocks
    vector<FileEntry> blockFES = readFolder("assets/blocks");
    blocks.add("air", new Block("air", nullptr));
    for(FileEntry fe : blockFES) {
        if(!fe.hasExtension("td"))
            continue;
        string filename = "assets/blocks/" + fe.name;
        DataEntry* de = DataEntry::readText(readFileString(filename.c_str()));
        if(de->type == DataEntry::ERROR) {
            de->prettyPrint(cout);
            continue;
        }
        fe.removeExtension(2);
        addBlockToRegistry(de, fe.name);
        delete de;
    }

    //cout << "REGISTRY:\n";
    //shaders.print(cout, "shaders");
    //blocks.print(cout, "blocks");
    //for(auto block : blocks.items)
    //    cout << block->name << ": " << (u16)block->solidity << "\n";

}