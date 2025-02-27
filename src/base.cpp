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

bool FileEntry::hasExtension(const string& ext) const {
    if(name.size() < ext.size()+1)
        return false;
    return name[name.size()-ext.size()-1] == '.' && 
        strcmp(&name[name.size()-ext.size()], &ext[0]) == 0;
}

void FileEntry::removeExtension(u32 extsize) {
    name.resize(name.size()-extsize-1);
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

void readFolderR(string& foldername, string& basename, vector<FileEntry>& out) {
    DIR* dir;
    struct dirent* entry;
    dir = opendir(foldername.c_str());
    if (dir == nullptr) return;
    entry = readdir(dir);
    while(entry != nullptr) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            entry = readdir(dir);
            continue;
        }
        if(entry->d_type == DT_DIR) {
            u64 sizes[] = { foldername.size(), basename.size() };
            foldername += "/";
            foldername += entry->d_name;
            basename += entry->d_name;
            basename += "/";
            readFolderR(foldername, basename, out);
            foldername.resize(sizes[0]);
            basename.resize(sizes[1]);
        }
        else
            out.push_back({ false, basename + entry->d_name });
        entry = readdir(dir);
    }
    closedir(dir);
}

vector<FileEntry> readFolderRecursively(const char* name) {
    vector<FileEntry> result;
    string foldername = name;
    string basename = "";
    readFolderR(foldername, basename, result);
    return result;
}

