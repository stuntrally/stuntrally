#include "core.h"


#ifdef SH_VERTEX_SHADER

SH_BEGIN_PROGRAM    // vertex

    shUniform(float3, eyePosition) @shAutoConstant(eyePosition, camera_position)
    shUniform(float4x4, wvp) @shAutoConstant(wvp, worldviewproj_matrix)
    shUniform(float4x4, wv)  @shAutoConstant(wv, worldview_matrix)
    //shOutput(float3, eyeVector)

	shVertexInput(float2, uv0)
	shOutput(float2, UV)

	shColourInput(float4)
	shOutput(float4, vertColor)
	shOutput(float, fade)
    
SH_START_PROGRAM  //  vert  ----
{
    shOutputPosition = shMatrixMult(wvp, shInputPosition);
    //eyeVector = shMatrixMult(wv, shInputPosition).xyz - eyePosition;

	UV = uv0;
	vertColor = colour;

	float dist = shOutputPosition.w;
	//fade = shSaturate((dist+0.5f)*0.1f) * shSaturate(1.f - pow(dist,1.5f)*0.002f);
	fade = shSaturate( (dist-5.f)*0.02f );  // par
}


#else  //  fragment

SH_BEGIN_PROGRAM

	shSampler2D(diffuseMap)
    //shInput(float3, eyeVector)
	shInput(float2, UV)
	shInput(float4, vertColor)
	shInput(float, fade)
    
	shUniform(float4, par)  @shAutoConstant(par, custom, 0)

SH_START_PROGRAM  //  frag  ----
{
    //float3 eyeVec = normalize(eyeVector);
    
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

    //shOutputColour(0).w = UV.x*UV.y; //tex.g;  //a
}

#endif
