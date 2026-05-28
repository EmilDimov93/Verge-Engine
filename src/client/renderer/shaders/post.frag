#version 450

layout(set = 0, binding = 0) uniform sampler2D prePostImage;

layout(push_constant) uniform PushPost {
    float vignetteStrength;
    float vignetteRadius;
} pushPost;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 base = texture(prePostImage, inUV);

    float distFromCenter = length(inUV - 0.5);
    float vignetteFactor = smoothstep(pushPost.vignetteRadius, pushPost.vignetteRadius - 0.5, distFromCenter);
    base *= mix(1.0, vignetteFactor, pushPost.vignetteStrength);

    outColor = base;
}