#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

layout(binding = 0) uniform UboViewProjection {
    mat4 projection;
    mat4 view;
} uboViewProjection;

layout(push_constant) uniform PushModel {
    mat4 model;
}pushModel;

layout(location = 0) out vec3 fragCol;

void main(){
    gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(pos, 1.0);

    fragCol = col;
}