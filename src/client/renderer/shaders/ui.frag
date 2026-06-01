#version 450

layout(location = 0) in vec4 inCol;
layout(location = 1) in vec2 fragTex;
layout(location = 2) flat in uint fragTextureIndex;

layout(location = 0) out vec4 outCol;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

void main()
{
    outCol = (fragTextureIndex == 0) ? inCol : texture(textureSampler, fragTex);
}