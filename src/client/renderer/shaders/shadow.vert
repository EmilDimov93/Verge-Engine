#version 450
layout(location = 0) in vec3 pos;

layout(push_constant) uniform ShadowPushData {
    mat4 model;
    mat4 lightSpaceMat;
} shadowPushData;

void main()
{
    gl_Position = shadowPushData.lightSpaceMat * shadowPushData.model * vec4(pos, 1.0);
}