#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 normal;

layout(set = 0, binding = 0) uniform UboViewProjection {
    mat4 projection;
    mat4 view;
    vec4 lightPos;
    vec4 viewPos;
} uboViewProjection;

layout(push_constant) uniform PushModel {
    mat4 model;
    uint textureIndex;
    float lightStrength;
}pushModel;

layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragTex;
layout(location = 2) flat out uint fragTextureIndex;
layout(location = 3) out vec3 fragWorldPos;
layout(location = 4) out vec3 fragNormal;
layout(location = 5) flat out float fragLightStrength;

void main(){
    vec4 worldPos = pushModel.model * vec4(pos, 1.0);
    gl_Position = uboViewProjection.projection * uboViewProjection.view * worldPos;

    fragCol = col;
    fragTex = tex;
    fragTextureIndex = pushModel.textureIndex;
    fragWorldPos = worldPos.xyz;
    fragNormal = mat3(pushModel.model) * normal;
    fragLightStrength = pushModel.lightStrength;
}