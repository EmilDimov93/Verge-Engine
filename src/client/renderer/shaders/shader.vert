#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) uniform UboCamera {
    mat4 projection;
    mat4 view;
    mat4 lightSpaceMat;
} uboCamera;

layout(push_constant) uniform PushVertex {
    mat4 model;
    uint textureIndex;
    float lightStrength;
}pushVertex;

layout(location = 0) out vec2 fragTex;
layout(location = 1) flat out uint fragTextureIndex;
layout(location = 2) out vec3 fragWorldPos;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) flat out float fragLightStrength;
layout(location = 5) out vec4 fragPosLightSpace;

void main(){
    vec4 worldPos = pushVertex.model * vec4(pos, 1.0);
    gl_Position = uboCamera.projection * uboCamera.view * worldPos;

    fragTex = tex;
    fragTextureIndex = pushVertex.textureIndex;
    fragWorldPos = worldPos.xyz;
    fragNormal = mat3(pushVertex.model) * normal;
    fragLightStrength = pushVertex.lightStrength;
    fragPosLightSpace = uboCamera.lightSpaceMat * pushVertex.model * vec4(pos, 1.0);
}