#version 450


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    vec4 sampled = texture(texSampler, fragTexCoord);
    vec4 color = vec4(fragColor, 1.0);
    outColor = sampled*0.8+color*0.2;
}