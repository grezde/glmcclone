#include "base.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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