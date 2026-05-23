#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;

layout(set = 0, binding = 0) uniform UboUI {
    mat4 orthographicProj;
} uboUI;

layout(push_constant) uniform PushUI {
    mat4 model;
}pushUI;

layout(location = 0) out vec4 outCol;

void main()
{
    gl_Position = uboUI.orthographicProj * pushUI.model * vec4(inPos.x, inPos.y, 0.0, 1.0);

    outCol = inCol;
}