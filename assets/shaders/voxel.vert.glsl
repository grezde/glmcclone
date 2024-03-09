#version 450

layout(location = 0) in uint pos_ao;
layout(location = 1) in uint texCoords;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 outTexCoord;

uniform ivec3 chunkCoords;
uniform mat4 view;
uniform mat4 proj;

void main() {
    uint z = pos_ao % 64;
    uint y = (pos_ao/64) % 64;
    uint x = (pos_ao/64/64) % 64;
    uint ao = (pos_ao/64/64/64) % 8;
    uint v = texCoords % 256;
    uint u = (texCoords/256) % 256;
    vec3 inPosition = vec3(0, y, z) + vec3(chunkCoords) * 32.0;
    gl_Position = proj * view * vec4(inPosition, 1.0);
    fragColor = vec3(1, 1, 1) * (1 - ao*0.06) * vec3(float(u), float(v), 16.0)/16.0;
    outTexCoord = vec2(u, v)/16;
}