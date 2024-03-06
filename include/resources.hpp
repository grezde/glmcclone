#pragma once
#include "base.hpp"
#include "data.hpp"
#include "game.hpp"
#include "renderer.hpp"
#include "world.hpp"

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

struct Texture {
    u32 id;
    string name;
};

namespace Registry {
    extern registry<u32> shaders;
    extern registry<u32> glTextures;

    extern registry<BlockModelConstructor> blockModels;
    extern registry<Texture> textures;
    extern registry<Block*> blocks;

    constexpr u32 ATLASDIM = 16;
    constexpr u32 ATLASTILE = 16;
    u32 makeAtlas(const string& folder); // Creates a texture atlas with all the files within the folder 
    void init();
};