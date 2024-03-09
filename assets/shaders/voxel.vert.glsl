#version 450

layout(location = 0) in uint pos_ao;
layout(location = 1) in uint texCoords;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 outTexCoord;

uniform ivec3 chunkCoords;
uniform mat4 view;
uniform mat4 proj;

const uint CHUNKSIZE = 32;
const uint ATLASDIM = 16;
const float AO_INTENSITY = 0.07;

void main() {
    uint x  = (pos_ao & 0x00003F);
    uint y  = (pos_ao & 0x000FC0) >> 6;
    uint z  = (pos_ao & 0x03F000) >> 12;
    uint ao = (pos_ao & 0x3C0000) >> 18;
    uint v = (texCoords & 0x00FF);
    uint u = (texCoords & 0xFF00) >> 8;
    
    vec3 inPosition = vec3(x, y, z) + vec3(chunkCoords) * 32.0;
    gl_Position = proj * view * vec4(inPosition, 1.0);
    fragColor = vec3(1, 1, 1) * (1 - ao * AO_INTENSITY);
    outTexCoord = vec2(u, v) / ATLASDIM;
}