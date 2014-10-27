//  RTT shader that computes
//  terrain blendmap with noise
//  (up to 4 layers in r,g,b,a, sum not lerp)
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

	shUniform(float, terrainWorldSize)  @shSharedParameter(terrainWorldSize)

	//  blendmap params for 4 layers
	shUniform(float4, Hmin)   @shUniformProperty4f(Hmin, Hmin)
	shUniform(float4, Hmax)   @shUniformProperty4f(Hmax, Hmax)
	shUniform(float4, Hsmt)   @shUniformProperty4f(Hsmt, Hsmt)
														 
	shUniform(float4, Amin)   @shUniformProperty4f(Amin, Amin)
	shUniform(float4, Amax)   @shUniformProperty4f(Amax, Amax)
	shUniform(float4, Asmt)   @shUniformProperty4f(Asmt, Asmt)
	shUniform(float4, Nonly)  @shUniformProperty4f(Nonly,Nonly)  // bool

	//  noise mul
	shUniform(float3, Nnext)   @shUniformProperty3f(Nnext, Nnext)
	shUniform(float3, Nprev)   @shUniformProperty3f(Nprev, Nprev)
	shUniform(float2, Nnext2)  @shUniformProperty2f(Nnext2,Nnext2)
	//  noise +1,-1 pars
	shUniform(float3, Nfreq)   @shUniformProperty3f(Nfreq, Nfreq)
	shUniform(float3, Noct)    @shUniformProperty3f(Noct,  Noct)  // uint8
	shUniform(float3, Npers)   @shUniformProperty3f(Npers, Npers)
	shUniform(float3, Npow)    @shUniformProperty3f(Npow,  Npow)
	//  noise +2 pars
	shUniform(float2, Nfreq2)  @shUniformProperty2f(Nfreq2, Nfreq2)
	shUniform(float2, Noct2)   @shUniformProperty2f(Noct2,  Noct2)
	shUniform(float2, Npers2)  @shUniformProperty2f(Npers2, Npers2)
	shUniform(float2, Npow2)   @shUniformProperty2f(Npow2,  Npow2)

SH_START_PROGRAM
{
	float2 uv1 = float2(uv.x, 1-uv.y);
	float2 tuv = uv1 * terrainWorldSize / 512.f;
	float h = shSample(samHMap, uv1).x;
	float a = shSample(samAng,  uv).x;
	
	//  ter ang,h ranges  (turned off if noise only)
	float l0a = Nonly.x < 0.1f ? 0.f : linRange(a, Amin.x, Amax.x, Asmt.x) * linRange(h, Hmin.x, Hmax.x, Hsmt.x), l0 = l0a;
	float l1a = Nonly.y < 0.1f ? 0.f : linRange(a, Amin.y, Amax.y, Asmt.y) * linRange(h, Hmin.y, Hmax.y, Hsmt.y), l1 = l1a;
	float l2a = Nonly.z < 0.1f ? 0.f : linRange(a, Amin.z, Amax.z, Asmt.z) * linRange(h, Hmin.z, Hmax.z, Hsmt.z), l2 = l2a;
	float l3a = Nonly.w < 0.1f ? 0.f : linRange(a, Amin.w, Amax.w, Asmt.w) * linRange(h, Hmin.w, Hmax.w, Hsmt.w), l3 = l3a;
	
	//  noise par
	float n0 = Nnext.x < 0.01f ? 0.f : Nnext.x * pow( snoise(tuv, Nfreq.x, int(Noct.x), Npers.x), Npow.x);
	float n1 = Nnext.y < 0.01f ? 0.f : Nnext.y * pow( snoise(tuv, Nfreq.y, int(Noct.y), Npers.y), Npow.y);
	float n2 = Nnext.z < 0.01f ? 0.f : Nnext.z * pow( snoise(tuv, Nfreq.z, int(Noct.z), Npers.z), Npow.z);

	float p1 = Nprev.x < 0.01f ? 0.f : Nprev.x * pow( snoise(tuv, Nfreq.x+3.f, int(Noct.x), Npers.x), Npow.x);
	float p2 = Nprev.y < 0.01f ? 0.f : Nprev.y * pow( snoise(tuv, Nfreq.y+3.f, int(Noct.y), Npers.y), Npow.y);
	float p3 = Nprev.z < 0.01f ? 0.f : Nprev.z * pow( snoise(tuv, Nfreq.z+3.f, int(Noct.z), Npers.z), Npow.z);

	float m0 = Nnext2.x < 0.01f ? 0.f : Nnext2.x * pow( snoise(tuv, Nfreq2.x, int(Noct2.x), Npers2.x), Npow2.x);
	float m1 = Nnext2.y < 0.01f ? 0.f : Nnext2.y * pow( snoise(tuv, Nfreq2.y, int(Noct2.y), Npers2.y), Npow2.y);

	//  add noise
	//  +1, to next layer
	l1 += l0a * n0;  l0 *= 1.f-n0;
	l2 += l1a * n1;  l1 *= 1.f-n1;
	l3 += l2a * n2;  l2 *= 1.f-n2;
	//  -1, to prev
	l0 += l1a * p1;  l1 *= 1.f-p1;
	l1 += l2a * p2;  l2 *= 1.f-p2;
	l2 += l3a * p3;  l3 *= 1.f-p3;
	//  +2
	l2 += l0a * m0;  l0 *= 1.f-m0;
	l3 += l1a * m1;  l1 *= 1.f-m1;
	
	//  normalize  (sum = 1)
	l0 = shSaturate(l0);  l1 = shSaturate(l1);  l2 = shSaturate(l2);  l3 = shSaturate(l3);  // fix white dots
	float ll = l0+l1+l2+l3;
	if (ll < 0.01f)  {  l0 = 1.f;  ll = l0+l1+l2+l3;  }  // fix black dots
	ll = 1/ll;  l0 *= ll;  l1 *= ll;  l2 *= ll;  l3 *= ll;  // norm
	shOutputColour(0) = float4(l0, l1, l2, l3);
}
#endif
