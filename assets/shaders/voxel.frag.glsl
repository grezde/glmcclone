#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

uniform sampler2D tex;

void main() {
    outColor = vec4(fragColor, 1.0) + texture(tex, texCoord);
}