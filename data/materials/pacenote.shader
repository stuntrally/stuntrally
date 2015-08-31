#include "core.h"


#ifdef SH_VERTEX_SHADER

SH_BEGIN_PROGRAM    // vertex

    shUniform(float3, eyePosition) @shAutoConstant(eyePosition, camera_position)
    shUniform(float4x4, wvp) @shAutoConstant(wvp, worldviewproj_matrix)

	shUniform(float3, paceParams)  @shSharedParameter(paceParams)

	shVertexInput(float2, uv0)
	shOutput(float2, UV)

	shColourInput(float4)
	shOutput(float4, vertColor)
	shOutput(float, fade)
    
SH_START_PROGRAM  //  vert  ----
{
	float4 pos = float4(shInputPosition.xyz * paceParams.x, shInputPosition.w);
    shOutputPosition = shMatrixMult(wvp, pos);

	UV = uv0;
	vertColor = colour;

	float dist = shOutputPosition.w;
	fade = shSaturate( (dist - paceParams.y ) * paceParams.z );
}

#else  //  fragment

SH_BEGIN_PROGRAM

	shSampler2D(diffuseMap)
	shInput(float2, UV)
	shInput(float4, vertColor)
	shInput(float, fade)
    
	shUniform(float4, par)  @shAutoConstant(par, custom, 0)

SH_START_PROGRAM  //  frag  ----
{
    float2 uv = UV.xy * 0.125f;
    if (par.x < 0.5f)  uv.x = 0.125f - uv.x;  // dir, mirror
    if (par.y > 0.f)  uv.x *= par.y;  // width mul
    uv += par.zw;  // offset
    
    float4 tex = shSample(diffuseMap, uv);

    float a = tex.a * vertColor.a * fade;
	if (a < 0.01f || tex.a < 0.5f)
		discard;

	shOutputColour(0) = tex * vertColor;
	shOutputColour(0).w *= fade;
}

#endif
