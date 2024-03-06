#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(/*set = 0, */ binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
} ubo;

void main() {
    vec4 pos1 = ubo.proj * ubo.view * ubo.model * (vec4(inPosition, 1.0) + 0.1 * vec4(
       sin(ubo.time*(2+gl_VertexID+2)), 
       cos(ubo.time*(1.5+0.8*2+gl_VertexID)),
       sin(ubo.time*(1+0.2*gl_VertexID) + 1), 0.0
    ));
    gl_Position = vec4(1.5*pos1.xy, pos1.zw);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
