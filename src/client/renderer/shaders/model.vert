#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) uniform UboCamera {
    mat4 projection;
    mat4 view;
} uboCamera;

layout(push_constant) uniform PushVertex {
    mat4 model;
}pushVertex;

layout(location = 0) out vec2 fragTex;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragNormal;

void main(){
    vec4 worldPos = pushVertex.model * vec4(pos, 1.);
    gl_Position = uboCamera.projection * uboCamera.view * worldPos;

    fragTex = tex;
    fragWorldPos = worldPos.xyz;
    fragNormal = normalize(mat3(pushVertex.model) * normal);
}