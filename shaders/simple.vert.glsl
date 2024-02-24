#version 450

layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inColor;
//layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
//layout(location = 1) out vec2 fragTexCoord;

//layout(binding = 0) uniform UniformBufferObject {
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//    float time;
//} ubo;

void main() {
    // ubo.proj * ubo.view * ubo.model *
    gl_Position = vec4(inPosition, 1.0);
    fragColor = vec3(1.0f, 1.0f, 1.0f);
    //fragTexCoord = inTexCoord;
}
