#include "resources.hpp"
#include "base.hpp"
#include "data.hpp"
#include <cstring>

Block::Block(const char* blockName, DataEntry* de) : name(blockName) {
    if(de->type != DataEntry::MAP)
        return;
    //for()
}

void Registry::init() {
    vector<FileEntry> textureFES = readFolder("assets/textures/blocks");
    for(FileEntry fe : textureFES) {
        cout << (fe.isDir ? "D" : "F") << " " << fe.name << "\n";
        
    }

    vector<FileEntry> blockFES = readFolder("assets/blocks");
    for(FileEntry fe : blockFES) {
        if(fe.name.size() < 3)
            continue;
        if(!(fe.name[fe.name.size()-3] == '.' && fe.name[fe.name.size()-2] == 't' && fe.name[fe.name.size()-1] == 'd'))
            continue;
        cout << (fe.isDir ? "D" : "F") << " " << fe.name << "\n";
        string filename = "assets/blocks/" + fe.name;
        DataEntry* de = DataEntry::readText(readFileString(filename.c_str()));
        char* blockName = (char*)malloc(fe.name.size()-2);
        memcpy(blockName, fe.name.data(), fe.name.size()-3);
        blockName[fe.name.size()-3] = '\0';
        blockNames[blockName] = blocks.size();
        blocks.push_back(Block(blockName, de));
        delete de;
    }
}