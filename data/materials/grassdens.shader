//  RTT shader that computes
//  grass density map with noise
//  (up to 4 channels in r,g,b,a)
#include "core.h"
#include "noise.h"
	

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

	shSampler2D(samHMap)
	shSampler2D(samAng)
	shSampler2D(samRoad)

	shUniform(float, terrainWorldSize)  @shSharedParameter(terrainWorldSize)

	//  blendmap params for 4 layers
	shUniform(float4, Hmin)   @shUniformProperty4f(Hmin, Hmin)
	shUniform(float4, Hmax)   @shUniformProperty4f(Hmax, Hmax)
	shUniform(float4, Hsmt)   @shUniformProperty4f(Hsmt, Hsmt)
														 
	shUniform(float4, Amin)   @shUniformProperty4f(Amin, Amin)
	shUniform(float4, Amax)   @shUniformProperty4f(Amax, Amax)
	shUniform(float4, Asmt)   @shUniformProperty4f(Asmt, Asmt)

	//  noise
	shUniform(float4, Nmul)   @shUniformProperty4f(Nmul, Nmul)
	shUniform(float4, Nfreq)  @shUniformProperty4f(Nfreq, Nfreq)
	shUniform(float4, Noct)   @shUniformProperty4f(Noct,  Noct)  // uint8
	shUniform(float4, Npers)  @shUniformProperty4f(Npers, Npers)
	shUniform(float4, Npow)   @shUniformProperty4f(Npow,  Npow)
	//  road fit
	shUniform(float2, Rofs)   @shUniformProperty2f(Rofs, Rofs)
	shUniform(float4, Rpow)   @shUniformProperty4f(Rpow, Rpow)

SH_START_PROGRAM
{
	float2 uv1 = float2(uv.x, 1-uv.y);
	float2 tuv = uv1 * terrainWorldSize / 512.f;
	float h = shSample(samHMap, uv1).x;
	float a = shSample(samAng,  uv).x;
	float rd = shSample(samRoad, uv + Rofs).x;

	//  ter ang,h ranges
	float l0 = linRange(a, Amin.x, Amax.x, Asmt.x) * linRange(h, Hmin.x, Hmax.x, Hsmt.x);
	float l1 = linRange(a, Amin.y, Amax.y, Asmt.y) * linRange(h, Hmin.y, Hmax.y, Hsmt.y);
	float l2 = linRange(a, Amin.z, Amax.z, Asmt.z) * linRange(h, Hmin.z, Hmax.z, Hsmt.z);
	float l3 = linRange(a, Amin.w, Amax.w, Asmt.w) * linRange(h, Hmin.w, Hmax.w, Hsmt.w);
	
	//  noise par
	float n0 = Nmul.x < 0.01f ? 0.f : Nmul.x * pow( snoise(tuv, Nfreq.x, int(Noct.x), Npers.x), Npow.x);
	float n1 = Nmul.y < 0.01f ? 0.f : Nmul.y * pow( snoise(tuv, Nfreq.y, int(Noct.y), Npers.y), Npow.y);
	float n2 = Nmul.z < 0.01f ? 0.f : Nmul.z * pow( snoise(tuv, Nfreq.z, int(Noct.z), Npers.z), Npow.z);
	float n3 = Nmul.w < 0.01f ? 0.f : Nmul.w * pow( snoise(tuv, Nfreq.w, int(Noct.w), Npers.w), Npow.w);

	l0 = shSaturate(l0 * (1.f-n0)) * pow(rd, Rpow.x);
	l1 = shSaturate(l1 * (1.f-n1)) * pow(rd, Rpow.y);
	l2 = shSaturate(l2 * (1.f-n2)) * pow(rd, Rpow.z);
	l3 = shSaturate(l3 * (1.f-n3)) * pow(rd, Rpow.w);
	
	shOutputColour(0) = float4(l0,l1,l2,l3);
}

#endif
