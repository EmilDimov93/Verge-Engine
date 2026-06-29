#version 450

#define MAX_SHADOW_MAPS 10

layout(location = 0) in vec2 fragTex;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;

layout(set = 0, binding = 1) uniform UboLighting {
    vec4 lightPositions[MAX_SHADOW_MAPS];
    vec4 lightColors[MAX_SHADOW_MAPS];
    mat4 lightSpaceMats[MAX_SHADOW_MAPS];
    vec4 viewPos;
    uint lightCount;
    float outdoorBrightness;
} uboLighting;

layout(push_constant) uniform PushMaterial {
    layout(offset = 64) vec4 baseColor;
    float metallic;
    float roughness;
    uint textureIndex;
} pushMaterial;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;
layout(set = 0, binding = 2) uniform sampler2DShadow shadowMaps[MAX_SHADOW_MAPS];

layout(location = 0) out vec4 outColor;

float calcShadow(uint lightIndex, vec3 worldPos) {
    vec4 posLightSpace = uboLighting.lightSpaceMats[lightIndex] * vec4(worldPos, 1.0);
    vec3 shadowMapCoords = posLightSpace.xyz / posLightSpace.w;
    shadowMapCoords.xy = shadowMapCoords.xy * 0.5 + 0.5;
    if (shadowMapCoords.z > 1.0 || shadowMapCoords.z < 0.0) return 0.0;
    float lightFactor = texture(shadowMaps[lightIndex], shadowMapCoords.xyz);
    return 1.0 - lightFactor;
}

void main(){
    vec4 base = pushMaterial.baseColor * texture(textureSampler, fragTex);

    vec3 baseRgb = base.rgb;
    vec3 viewDir = normalize(uboLighting.viewPos.xyz - fragWorldPos);

    vec3 ambient = baseRgb * mix(0.02, 0.15, uboLighting.outdoorBrightness);

    vec3 lit = vec3(0.0);

    for (uint i = 1; i < uboLighting.lightCount; i++) {
        vec3 lightColor = uboLighting.lightColors[i].xyz;
        float intensity = uboLighting.lightColors[i].w;
        vec3 lightDir = normalize(uboLighting.lightPositions[i].xyz - fragWorldPos);
        vec3 halfVector = normalize(lightDir + viewDir);

        float diff = max(dot(fragNormal, lightDir), 0.);
        vec3 diffuse = baseRgb * lightColor * diff * intensity;

        float spec = pow(max(dot(fragNormal, halfVector), 0.), 32.);
        vec3 reflectance = mix(vec3(0.04), baseRgb, pushMaterial.metallic);
        vec3 specular = reflectance * lightColor * spec * intensity;

        float shadow = calcShadow(i, fragWorldPos);
        lit += (diffuse + specular) * (1.0 - shadow);
    }

    outColor = vec4(ambient + lit, base.a);
}