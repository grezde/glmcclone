#pragma once
#include "base.hpp"
#include "data.hpp"
#include "game.hpp"
#include "renderer.hpp"

struct BlockModel {
    virtual void addmesh() = 0;
};

struct NoModel : BlockModel {
    virtual void addmesh() {};
};
BlockModel* noModelConstructor(DataEntry* de);

struct CubeModel : BlockModel {
    u32 faces[6] = {};
    virtual void addmesh() {};
};
BlockModel* cubeModelConstructor(DataEntry* de);

typedef BlockModel* (*BlockModelConstructor)(DataEntry* de);

struct Block {
    u8 solidity;
    BlockModel* model;
    string name;
    Block(string blockName, DataEntry* de);
};

struct Texture {
    u32 id;
    string name;
};

template<typename T>
struct registry {
    vector<T> items;
    map<string, u32> names;

    inline bool has(const string& name) const { return names.find(name) != names.end(); }
    inline T& operator[](const string& name) { return items[names[name]]; }
    inline const T& operator[](const string& name) const { return items[names.at(name)]; }
    inline void add(const string& name, const T& value) { names[name] = items.size(); items.push_back(value); }

    void print(std::ostream& out, const string& name) {
        out << "----------------\n" << name << ":\n";
        for(auto p : names)
            out << p.first << ": index " << p.second << "\n";
    }
};

namespace Registry {
    extern registry<u32> shaders;
    extern registry<u32> glTextures;

    extern registry<BlockModelConstructor> blockModels;
    extern registry<Texture> textures;
    extern registry<Block*> blocks;

    void makeAtlas();
    void init();
};