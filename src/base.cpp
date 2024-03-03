#include "base.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


vector<u8> readFileBytes(const char* filename) {
    vector<u8> result;
    FILE *f = fopen(filename, "rb");
    if(!f) return result;
    fseek(f, 0, SEEK_END);
    u32 size = (u32)ftell(f);
    fseek(f, 0, SEEK_SET);
    result.resize(size);
    fread(result.data(), size, 1, f);
    fclose(f);
    return result;
}

string readFileString(const char* filename) {
    string result;
    FILE *f = fopen(filename, "rb");
    if(!f) return result;
    fseek(f, 0, SEEK_END);
    u32 size = (u32)ftell(f);
    fseek(f, 0, SEEK_SET);
    result.resize(size);
    fread(&result[0], size, 1, f);
    fclose(f);
    return result;
}

#ifdef WIN32
    #include <io.h>
    #define F_OK 0
    #define access _access
#else
#endif

bool fileExists(const char* filename) {
    return access(filename, F_OK) == 0;
}

#include <dirent.h>

vector<FileEntry> readFolder(const char* name) {
    DIR* dir;
    struct dirent* entry;
    dir = opendir(name);
    if (dir == nullptr)
        return vector<FileEntry>();
    vector<FileEntry> result;
    entry = readdir(dir);
    while(entry != nullptr) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            entry = readdir(dir);
            continue;
        }
        result.push_back({ entry->d_type == DT_DIR, entry->d_name });
        entry = readdir(dir);
    }
    closedir(dir);
    return result;
}
