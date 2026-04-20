#version 450

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragTex;
layout(location = 2) flat in uint fragTextureIndex;
layout(location = 3) in vec3 fragWorldPos;
layout(location = 4) in vec3 fragNormal;
layout(location = 5) flat in float fragLightStrength;

layout(set = 0, binding = 0) uniform UboViewProjection {
    mat4 projection;
    mat4 view;
    vec4 lightPos;
    vec4 viewPos;
} uboVP;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

void main(){
    vec3 base = (fragTextureIndex == 0) ? fragCol : texture(textureSampler, fragTex).rgb;

    if (fragLightStrength > 0.0) {
        outColor = vec4(1.0);
        return;
    }

    if (uboVP.lightPos.w == 0) {
        outColor = vec4(base, 1.0);
        return;
    }

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(uboVP.lightPos.xyz - fragWorldPos);
    vec3 V = normalize(uboVP.viewPos.xyz  - fragWorldPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    float intensity = uboVP.lightPos.w;
    vec3 color = base * (0.1 + 0.7 * diff * intensity) + vec3(0.3) * spec * intensity;
    outColor = vec4(color, 1.0);
}