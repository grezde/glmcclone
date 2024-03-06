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

u32 Registry::makeAtlas(const string& folder) {
    // u32 is the color
    u32* data = (u32*)malloc(4*ATLASTILE*ATLASTILE*ATLASDIM*ATLASDIM);
    memset(data, 0, 4*ATLASTILE*ATLASTILE*ATLASDIM*ATLASDIM);
    for(u32 y=0; y<ATLASTILE; y++) for(u32 x=0; x<ATLASTILE; x++) {
        u32 mag = (x < 8) ^ (y < 8);
        data[y*ATLASTILE*ATLASDIM+x] = mag ? 0xFFFF00FF : 0xFF000000;
    }
    u32 index = 1;
    for(auto& p : textures.names) {
        if(folder.size() != 0 && p.first.compare(0, folder.size(), folder) != 0)
            continue;
        if(p.first.size() < folder.size()+2)
            continue;
        if(index == ATLASTILE*ATLASTILE)
            ERR_EXIT("Atlas too small");
        string filename = "assets/textures/" + p.first + ".png";
        i32 imageWidth, imageHeight, imageChannels;
        u8* newdata = stbi_load(filename.c_str(), &imageWidth, &imageHeight, &imageChannels, 4);
        cout << "ATLAS " << p.first << " at " << filename << ": " << imageChannels << " " << imageWidth << " " << imageHeight << "\n";
        if(data == nullptr) ERR_EXIT("Could not load image " << p.first << ", " << filename);
        if(imageChannels != 4 || imageWidth != ATLASTILE || imageHeight != ATLASTILE)
            ERR_EXIT("Image not in apropriate format " << p.first);
        for(u32 y=0; y < ATLASTILE; y++)
        for(u32 x=0; x < ATLASTILE; x++) {
            data[((index/ATLASDIM)*ATLASTILE + y)*ATLASTILE*ATLASDIM + (index%ATLASDIM)*ATLASTILE + x] = ((u32*)newdata)[ATLASTILE*y+x];
        };
        stbi_image_free(newdata);
        index++;
    }
    #ifdef DEBUG
        stbi_write_png("output/atlas_new.png", ATLASTILE*ATLASDIM, ATLASTILE*ATLASDIM, 4, data, ATLASTILE*ATLASDIM*4);
    #endif
    // Finally we must flip everything in acordance to OpenGL
    for(u32 y=0; y<ATLASTILE*ATLASDIM/2; y++) for(u32 x=0; x<ATLASTILE*ATLASDIM; x++) {
        u32 temp = data[y*ATLASTILE*ATLASDIM+x];
        data[y*ATLASTILE*ATLASDIM+x] = data[(ATLASDIM*ATLASTILE-y-1)*ATLASTILE*ATLASDIM+x];
        data[(ATLASDIM*ATLASTILE-y-1)*ATLASTILE*ATLASDIM+x] = temp;
    };
    u32 t = gl::textureFromMemory((u8*)data, ATLASDIM*ATLASTILE, ATLASDIM*ATLASTILE, 4);
    free(data);
    return t;
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
    textures.add("missing", { 0, "missing" });
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
    blocks.add("air", new Block("air", nullptr));
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
    glTextures.add("blockAtlas", makeAtlas("blocks"));

}