#version 450

layout(set = 0, binding = 0) uniform sampler2D prePostImage;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 base = texture(prePostImage, inUV);
    outColor = vec4(base.r + 0.01, base.g + 0.005, base.ba);
}