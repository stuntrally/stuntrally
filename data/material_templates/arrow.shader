#include "core.h"


#define FRESNEL_SCALE 0.5
#define FRESNEL_POWER 1


#ifdef SH_VERTEX_SHADER

SH_BEGIN_PROGRAM

    shUniform(float3, eyePosition) @shAutoConstant(eyePosition, camera_position)

    shUniform(float4x4, wvp) @shAutoConstant(wvp, worldviewproj_matrix)
    shUniform(float4x4, wv)  @shAutoConstant(wv, worldview_matrix)
    
    
    shNormalInput(float3)
    shOutput(float3, normalPassthrough)
    
    shOutput(float3, eyeVector)

SH_START_PROGRAM
{

    eyeVector = shMatrixMult(wv, shInputPosition).xyz - eyePosition;

    shOutputPosition = shMatrixMult(wvp, shInputPosition);

    normalPassthrough = normal;
}

#else

SH_BEGIN_PROGRAM

    shInput(float3, normalPassthrough)
    shInput(float3, eyeVector)
    
    shUniform(float3, arrowColour1)     @shSharedParameter(arrowColour1)
    shUniform(float3, arrowColour2)     @shSharedParameter(arrowColour2)

SH_START_PROGRAM
{

    float3 eyeVec = normalize(eyeVector);
    float3 normal = normalize(normalPassthrough);
    
    float colorFactor = FRESNEL_SCALE * pow (1 + dot(eyeVec, normal), FRESNEL_POWER);
    colorFactor = min(colorFactor, 1);
    
    shOutputColour(0).xyz = shLerp(arrowColour1, arrowColour2, colorFactor);

    shOutputColour(0).w = 0.5;

}

#endif
