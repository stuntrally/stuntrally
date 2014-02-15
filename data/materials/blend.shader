//  RTT shader that computes
//  terrain blendmap with noise
//  (up to 4 layers in r,g,b,a, sum not lerp)
#include "core.h"
#include "noise.h"


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

	shUniform(float3, Nnext)   @shUniformProperty3f(Nnext, Nnext)
	shUniform(float3, Nprev)   @shUniformProperty3f(Nprev, Nprev)
	shUniform(float2, Nnext2)  @shUniformProperty2f(Nnext2, Nnext2)

	shSampler2D(samHMap)
	shSampler2D(samAng)

SH_START_PROGRAM
{
	float2 uv1 = float2(uv.x, 1-uv.y);
	float h = shSample(samHMap, uv1);
	float a = shSample(samAng,  uv);
	
	//  ter ang,h ranges
	float l0a = linRange(a, Amin.x, Amax.x, Asmt.x) * linRange(h, Hmin.x, Hmax.x, Hsmt.x), l0 = l0a;
	float l1a = linRange(a, Amin.y, Amax.y, Asmt.y) * linRange(h, Hmin.y, Hmax.y, Hsmt.y), l1 = l1a;
	float l2a = linRange(a, Amin.z, Amax.z, Asmt.z) * linRange(h, Hmin.z, Hmax.z, Hsmt.z), l2 = l2a;
	float l3a = linRange(a, Amin.w, Amax.w, Asmt.w) * linRange(h, Hmin.w, Hmax.w, Hsmt.w), l3 = l3a;
	
	//float n0 = pow( snoise(uv1, 0.006f, 3, 0.25f) * snoise(uv1, 0.02f, 3, 0.3f) * snoise(uv1, 0.05f, 3, 0.4f), 2.f);
	//float n = lerp( snoise(uv1, 0.02f, 3, 0.3f), snoise(uv1, 0.01f, 4, 0.3f), snoise(uv1, 0.03f, 4, 0.2f) );

	//  noise par
	float n0 = Nnext.x < 0.01f ? 0.f : Nnext.x * snoise(uv1, 0.0121f, 3, 0.27f);  // par.. freq, oct, pers, pow
	float n1 = Nnext.y < 0.01f ? 0.f : Nnext.y * snoise(uv1, 0.0135f, 3, 0.30f);
	float n2 = Nnext.z < 0.01f ? 0.f : Nnext.z * snoise(uv1, 0.0130f, 3, 0.31f);

	float p1 = Nprev.x < 0.01f ? 0.f : Nprev.x * snoise(uv1, 0.0126f, 3, 0.39f);
	float p2 = Nprev.y < 0.01f ? 0.f : Nprev.y * snoise(uv1, 0.0131f, 3, 0.32f);
	float p3 = Nprev.z < 0.01f ? 0.f : Nprev.z * snoise(uv1, 0.0123f, 3, 0.27f);

	float nn0 = Nnext2.x < 0.01f ? 0.f : Nnext2.x * snoise(uv1, 0.0186f, 3, 0.35f);
	float nn1 = Nnext2.y < 0.01f ? 0.f : Nnext2.y * snoise(uv1, 0.0191f, 3, 0.34f);

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
	l2 += l0a * nn0;  l0 *= 1.f-nn0;
	l3 += l1a * nn1;  l1 *= 1.f-nn1;
	
	//  normalize  (sum = 1)
	l0 = shSaturate(l0);  l1 = shSaturate(l1);  l2 = shSaturate(l2);  l3 = shSaturate(l3);  // fix white dots
	float ll = l0+l1+l2+l3;
	if (ll < 0.01f)  {  l0 = 1.f;  ll = l0+l1+l2+l3;  }  // fix black dots
	ll = 1/ll;  l0 *= ll;  l1 *= ll;  l2 *= ll;  l3 *= ll;  // norm
	shOutputColour(0) = float4(l0, l1, l2, l3);
}
#endif
