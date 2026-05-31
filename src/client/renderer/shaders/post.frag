#version 450

layout(set = 0, binding = 0) uniform sampler2D prePostImage;

const uint POST_EFFECT_DITHERING_BIT = 1u << 0;
layout(push_constant) uniform PushPost {
    float vignetteStrength;
    float vignetteRadius;
    uint flags;
} pushPost;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

const float colorLevels = 255.0;

const float bayerMatrix4x4[16] = float[16](
     0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
    12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0,
     3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
    15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0
);

void main()
{
    vec4 base = texture(prePostImage, inUV);

    float distFromCenter = length(inUV - 0.5);
    float vignetteFactor = 1.0 - smoothstep(pushPost.vignetteRadius - 0.5, pushPost.vignetteRadius, distFromCenter);
    base *= mix(1.0, vignetteFactor, pushPost.vignetteStrength);

    if((pushPost.flags & POST_EFFECT_DITHERING_BIT) != 0u)
    {
        int matrixIndex = (int(gl_FragCoord.x) % 4) + (int(gl_FragCoord.y) % 4) * 4;
        vec3 ditheredColor = base.xyz + (bayerMatrix4x4[matrixIndex] - 0.5) / colorLevels;
        base = vec4(floor(ditheredColor * colorLevels + 0.5) / colorLevels, base.a);
    }

    outColor = base;
}