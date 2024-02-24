#include "base.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

vector<u8> readWholeFile(const char* filename) {
    vector<u8> result;
    FILE *f = fopen(filename, "rb");
    if(!f) return result;
    fseek(f, 0, SEEK_END);
    u32 size = (u32)ftell(f);
    fseek(f, 0, SEEK_SET);
    result.resize(size+1);
    fread(result.data(), size, 1, f);
    result[result.size()-1] = 0;
    fclose(f);
    return result;
}