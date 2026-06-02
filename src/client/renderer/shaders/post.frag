#version 450

layout(set = 0, binding = 0) uniform sampler2D prePostImage;

const uint POST_EFFECT_FXAA_BIT = 1u << 0;
const uint POST_EFFECT_DITHERING_BIT = 1u << 1;
layout(push_constant) uniform PushPost {
    float vignetteStrength;
    float vignetteRadius;
    uint flags;
} pushPost;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

float rgbToLuma(vec3 linearColor)
{
    return sqrt(dot(linearColor, vec3(0.299, 0.587, 0.114)));
}

vec3 applyFxaa(sampler2D sourceImage, vec2 sampleUV, vec2 inverseScreenSize)
{
    const int FXAA_ITERATIONS = 12;
    const float FXAA_SUBPIXEL_QUALITY = 1.0;
    const float FXAA_EDGE_THRESHOLD_MIN = 0.0156;
    const float FXAA_EDGE_THRESHOLD_MAX = 0.063;

    const float fxaaQualityStep[12] = float[12](
        1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0
    );

    vec3 colorCenter = texture(sourceImage, sampleUV).rgb;

    float lumaCenter = rgbToLuma(colorCenter);
    float lumaDown = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2( 0, -1)).rgb);
    float lumaUp = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2( 0,  1)).rgb);
    float lumaLeft = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2(-1,  0)).rgb);
    float lumaRight = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2( 1,  0)).rgb);

    float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));
    float lumaRange = lumaMax - lumaMin;

    if (lumaRange < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD_MAX))
    {
        return colorCenter;
    }

    float lumaDownLeft  = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2(-1, -1)).rgb);
    float lumaUpRight = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2( 1,  1)).rgb);
    float lumaUpLeft = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2(-1,  1)).rgb);
    float lumaDownRight = rgbToLuma(textureOffset(sourceImage, sampleUV, ivec2( 1, -1)).rgb);

    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;
    float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaDownRight + lumaUpRight;
    float lumaUpCorners = lumaUpRight + lumaUpLeft;

    float edgeHorizontal =
        abs(-2.0 * lumaLeft + lumaLeftCorners) +
        abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 +
        abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical =
        abs(-2.0 * lumaUp + lumaUpCorners) +
        abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 +
        abs(-2.0 * lumaDown + lumaDownCorners);

    bool isHorizontal = (edgeHorizontal >= edgeVertical);

    float luma1 = isHorizontal ? lumaDown : lumaLeft;
    float luma2 = isHorizontal ? lumaUp : lumaRight;
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

    float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

    float lumaLocalAverage = 0.0;
    if (is1Steepest)
    {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    }
    else
    {
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
    }

    vec2 currentUV = sampleUV;
    if (isHorizontal)
        currentUV.y += stepLength * 0.5;
    else
        currentUV.x += stepLength * 0.5;

    vec2 edgeOffset = isHorizontal ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);

    vec2 uvNegative = currentUV - edgeOffset;
    vec2 uvPositive = currentUV + edgeOffset;

    float lumaEndNegative = rgbToLuma(texture(sourceImage, uvNegative).rgb) - lumaLocalAverage;
    float lumaEndPositive = rgbToLuma(texture(sourceImage, uvPositive).rgb) - lumaLocalAverage;

    bool reachedNegative = abs(lumaEndNegative) >= gradientScaled;
    bool reachedPositive = abs(lumaEndPositive) >= gradientScaled;
    bool reachedBoth = reachedNegative && reachedPositive;

    if (!reachedNegative) uvNegative -= edgeOffset;
    if (!reachedPositive) uvPositive += edgeOffset;

    if (!reachedBoth)
    {
        for (int i = 2; i < FXAA_ITERATIONS; i++)
        {
            if (!reachedNegative)
                lumaEndNegative = rgbToLuma(texture(sourceImage, uvNegative).rgb) - lumaLocalAverage;
            if (!reachedPositive)
                lumaEndPositive = rgbToLuma(texture(sourceImage, uvPositive).rgb) - lumaLocalAverage;

            reachedNegative = abs(lumaEndNegative) >= gradientScaled;
            reachedPositive = abs(lumaEndPositive) >= gradientScaled;
            reachedBoth = reachedNegative && reachedPositive;

            if (!reachedNegative) uvNegative -= edgeOffset * fxaaQualityStep[i];
            if (!reachedPositive) uvPositive += edgeOffset * fxaaQualityStep[i];

            if (reachedBoth) break;
        }
    }

    float distanceNegative = isHorizontal ? (sampleUV.x - uvNegative.x) : (sampleUV.y - uvNegative.y);
    float distancePositive = isHorizontal ? (uvPositive.x - sampleUV.x) : (uvPositive.y - sampleUV.y);

    bool isNegativeNearest = distanceNegative < distancePositive;
    float distanceNearest = min(distanceNegative, distancePositive);
    float edgeThickness = distanceNegative + distancePositive;
    float pixelOffset = -distanceNearest / edgeThickness + 0.5;

    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
    bool correctVariationNegative = (lumaEndNegative < 0.0) != isLumaCenterSmaller;
    bool correctVariationPositive = (lumaEndPositive < 0.0) != isLumaCenterSmaller;
    bool correctVariation = isNegativeNearest ? correctVariationNegative : correctVariationPositive;

    float finalOffset = correctVariation ? pixelOffset : 0.0;

    float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * FXAA_SUBPIXEL_QUALITY;

    finalOffset = max(finalOffset, subPixelOffsetFinal);

    vec2 finalUV = sampleUV;
    if (isHorizontal)
        finalUV.y += finalOffset * stepLength;
    else
        finalUV.x += finalOffset * stepLength;

    return texture(sourceImage, finalUV).rgb;
}

vec3 applyDithering(vec3 base)
{
    const float colorLevels = 255.0;

    const float bayerMatrix4x4[16] = float[16](
        0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
        12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0,
        3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
        15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0
    );

    int matrixIndex = (int(gl_FragCoord.x) % 4) + (int(gl_FragCoord.y) % 4) * 4;
    vec3 ditheredColor = base.xyz + (bayerMatrix4x4[matrixIndex] - 0.5) / colorLevels;
    return floor(ditheredColor * colorLevels + 0.5) / colorLevels;
}

void main()
{
    vec4 base = texture(prePostImage, inUV);

    if((pushPost.flags & POST_EFFECT_FXAA_BIT) != 0u)
    {
        vec2 inverseScreenSize = 1.0 / vec2(textureSize(prePostImage, 0));
        base = vec4(applyFxaa(prePostImage, inUV, inverseScreenSize), base.a);
    }

    float distFromCenter = length(inUV - 0.5);
    float vignetteFactor = 1.0 - smoothstep(pushPost.vignetteRadius - 0.5, pushPost.vignetteRadius, distFromCenter);
    base *= mix(1.0, vignetteFactor, pushPost.vignetteStrength);

    if((pushPost.flags & POST_EFFECT_DITHERING_BIT) != 0u)
    {
        base = vec4(applyDithering(base.xyz), base.a);
    }

    outColor = base;
}