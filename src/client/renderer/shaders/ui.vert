#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;
layout(location = 2) in vec2 inTex;

layout(set = 0, binding = 0) uniform UboUI {
    mat4 orthographicProj;
} uboUI;

layout(push_constant) uniform PushUI {
    mat4 model;
    uint textureIndex;
}pushUI;

layout(location = 0) out vec4 outCol;
layout(location = 1) out vec2 fragTex;
layout(location = 2) flat out uint fragTextureIndex;

void main()
{
    gl_Position = uboUI.orthographicProj * pushUI.model * vec4(inPos.x, inPos.y, 0.0, 1.0);

    fragTex = inTex;
    fragTextureIndex = pushUI.textureIndex;

    outCol = inCol;
}