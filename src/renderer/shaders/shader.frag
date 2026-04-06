#version 450

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragTex;
layout(location = 2) flat in uint fragTextureIndex;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

void main(){
    if (fragTextureIndex == 0)
        outColor = vec4(fragCol, 1.0);
    else
        outColor = texture(textureSampler, fragTex);
}