#version 450

layout(location = 0) flat in vec4 fragCol;
layout(location = 1) in vec2 fragTex;
layout(location = 2) flat in uint fragTextureIndex;
layout(location = 3) in vec3 fragWorldPos;
layout(location = 4) in vec3 fragNormal;
layout(location = 5) flat in float fragLightStrength;
layout(location = 6) in vec4 fragPosLightSpace;

layout(set = 0, binding = 1) uniform UboLighting {
    vec4 lightPos;
    vec4 lightColor;
    vec4 viewPos;
    float outdoorBrightness;
} uboLighting;

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
    vec4 base = (fragTextureIndex == 0) ? fragCol : texture(textureSampler, fragTex);

    if (fragLightStrength > 0.0) {
        outColor = uboLighting.lightColor;
        return;
    }

    if (uboLighting.lightPos.w == 0) {
        outColor = base;
        return;
    }

    vec3 normalDir = normalize(fragNormal);
    vec3 lightDir = normalize(uboLighting.lightPos.xyz - fragWorldPos);
    vec3 viewDirection = normalize(uboLighting.viewPos.xyz  - fragWorldPos);
    vec3 halfVector = normalize(lightDir + viewDirection);

    float diff = max(dot(normalDir, lightDir), 0.0);
    float spec = pow(max(dot(normalDir, halfVector), 0.0), 32.0);

    float intensity = uboLighting.lightPos.w;
    vec3 baseRgb = base.rgb;
    vec3 ambient = baseRgb * mix(0.02, 0.3, uboLighting.outdoorBrightness);
    vec3 diffuse = baseRgb * uboLighting.lightColor.rgb * diff * intensity * 0.5;
    vec3 specular = uboLighting.lightColor.rgb * spec * intensity * 0.3;
    vec3 color = ambient + (diffuse + specular) * (1.0 - calcShadow(fragPosLightSpace));
    outColor = vec4(color, base.a);
}