#pragma once
#include "base.hpp"
#include "data.hpp"
#include "game.hpp"
#include "renderer.hpp"
#include "world.hpp"
#include "entity.hpp"

template<typename T, typename keyT = std::string, typename mapT = std::map<keyT, u32>>
struct registry {
    vector<T> items;
    mapT names;

    inline bool has(const keyT& name) const { return names.find(name) != names.end(); }
    inline T& operator[](const keyT& name) { return items[names.at(name)]; }
    inline const T& operator[](const keyT& name) const { return items[names.at(name)]; }
    inline u32 add(const keyT& name, const T& value) { names[name] = items.size(); items.push_back(value); return items.size()-1; }
    
    void print(std::ostream& out, const string& name) {
        out << "----------------\n" << name << ":\n";
        for(auto p : names)
            out << p.first << ": index " << p.second << "\n";
    }
};

struct TextureRI {
    u32 id;
    enum UsageType {
        UNUSED,
        ATLAS,
        GLTEXTURE,
        GUI
    };
    UsageType usage = UNUSED;
    u32 usageID = 0;
    string name;
};

struct GLTexture {
    u32 glid;
    u32 width, height;
    string name;
};

struct BlockModelRI {
    typedef BlockModel* (*Constructor)(DataEntry* de);
    Constructor constructor;
    u32 defaultSolidity;
    string name;
};

struct EntityModelRI {
    typedef EntityModel* (*Constructor)(DataEntry* de);
    Constructor constructor;
    string name;
};

namespace Registry {
    extern registry<vector<string>> enums;
    extern registry<u32> shaders;
    extern registry<GLTexture> glTextures;
    extern registry<TextureRI> textures;

    extern registry<BlockModelRI> blockModels;
    extern registry<Block*> blocks;
    extern registry<EntityModelRI> entityModels;
    extern registry<EntityType> entities;

    constexpr u32 ATLASDIM = 16;
    constexpr u32 ATLASTILE = 16;
    u32* makeAtlas(); // Creates a texture atlas with all the files within the folder 
    void addToAtlas(u32* atlasData, u32& index, const string& folder);
    GLTexture finishAtlas(u32* atlasData);
    void makeGLTextures(const string& folder);
    void init();
};