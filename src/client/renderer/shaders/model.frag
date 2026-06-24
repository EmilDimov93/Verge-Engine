#version 450

layout(location = 0) in vec2 fragTex;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragPosLightSpace;
layout(location = 4) in float fragLightIntensity;

layout(set = 0, binding = 1) uniform UboLighting {
    vec4 lightPos;
    vec3 lightColor;
    float lightIntensity;
    vec4 viewPos;
    float outdoorBrightness;
} uboLighting;

layout(push_constant) uniform PushMaterial {
    layout(offset = 80) vec4 baseColor;
    float metallic;
    float roughness;
    uint textureIndex;
} pushMaterial;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;
layout(set = 0, binding = 2) uniform sampler2DShadow shadowMap;

layout(location = 0) out vec4 outColor;

float calcShadow(vec4 posLightSpace) {
    vec3 shadowMapCoords = posLightSpace.xyz / posLightSpace.w;
    shadowMapCoords.xy = shadowMapCoords.xy * 0.5 + 0.5;
    if (shadowMapCoords.z > 1.0 || shadowMapCoords.z < 0.0) return 0.0;
    float currentDepth = shadowMapCoords.z;
    float lightFactor = texture(shadowMap, vec3(shadowMapCoords.xy, currentDepth));
    return 1.0 - lightFactor;
}

void main(){
    vec4 base = pushMaterial.baseColor * texture(textureSampler, fragTex);

    if (fragLightIntensity > 0.) {
        outColor = vec4(uboLighting.lightColor, 1.);
        return;
    }

    vec3 lightDir = normalize(uboLighting.lightPos.xyz - fragWorldPos);
    vec3 viewDir = normalize(uboLighting.viewPos.xyz  - fragWorldPos);
    vec3 halfVector = normalize(lightDir + viewDir);

    vec3 baseRgb = base.rgb;

    vec3 ambient = baseRgb * mix(0.02, 0.3, uboLighting.outdoorBrightness);

    float diff = max(dot(fragNormal, lightDir), 0.);
    vec3 diffuse = baseRgb * uboLighting.lightColor * diff * uboLighting.lightIntensity * 0.5;

    float spec = pow(max(dot(fragNormal, halfVector), 0.), 32.);
    vec3 specular = uboLighting.lightColor * spec * uboLighting.lightIntensity * 0.3;

    outColor = vec4(ambient + (diffuse + specular) * (1. - calcShadow(fragPosLightSpace)), base.a);
}