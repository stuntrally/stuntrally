#include "core.h"


float linRange(float x, float xa, float xb, float s)  // val, min, max, smooth range
{
	if (x <= xa-s || x >= xb+s)  return 0.f;
	if (x >= xa && x <= xb)  return 1.f;
	if (x < xa)  return (x-xa)/s+1;
	if (x > xb)  return (xb-x)/s+1;
	return 0.f;
}
	

#ifdef SH_VERTEX_SHADER

//  vertex  ...........

SH_BEGIN_PROGRAM

	shUniform(float4x4, wvp)	@shAutoConstant(wvp, worldviewproj_matrix)
	shVertexInput(float2, uv0)

	shOutput(float2, uv)
	shOutput(float4, position)

SH_START_PROGRAM
{
	uv = uv0;
	shOutputPosition = shMatrixMult(wvp, shInputPosition);
}

#else

//  fragment  ...........

SH_BEGIN_PROGRAM

	shInput(float2, uv)

	//  blendmap params for 4 layers
	shUniform(float4, Hmin)   @shUniformProperty4f(Hmin, Hmin)
	shUniform(float4, Hmax)   @shUniformProperty4f(Hmax, Hmax)
	shUniform(float4, Hsmt)   @shUniformProperty4f(Hsmt, Hsmt)
														 
	shUniform(float4, Amin)   @shUniformProperty4f(Amin, Amin)
	shUniform(float4, Amax)   @shUniformProperty4f(Amax, Amax)
	shUniform(float4, Asmt)   @shUniformProperty4f(Asmt, Asmt)

	shSampler2D(samHMap)
	shSampler2D(samAng)

SH_START_PROGRAM
{
	float2 uv1 = float2(uv.x, 1-uv.y);
	float h = shSample(samHMap, uv1);
	float a = shSample(samAng,  uv);
	
	float l0 = linRange(a, Amin.x, Amax.x, Asmt.x) * linRange(h, Hmin.x, Hmax.x, Hsmt.x);
	float l1 = linRange(a, Amin.y, Amax.y, Asmt.y) * linRange(h, Hmin.y, Hmax.y, Hsmt.y);
	float l2 = linRange(a, Amin.z, Amax.z, Asmt.z) * linRange(h, Hmin.z, Hmax.z, Hsmt.z);
	float l3 = linRange(a, Amin.w, Amax.w, Asmt.w) * linRange(h, Hmin.w, Hmax.w, Hsmt.w);
	
	//  normalize
	float ll = max(0.01f, l0+l1+l2+l3);
	l0 /= ll;  l1 /= ll;  l2 /= ll;  l3 /= ll;
	shOutputColour(0) = float4(l0, l1, l2, l3);

}

#endif
