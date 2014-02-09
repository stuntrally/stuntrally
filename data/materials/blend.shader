#include "core.h"


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

	#if 0
	//  blendmap params for 4 layers
	shUniform(float4, Hmin)   @shSharedParameter(Hmin)
	shUniform(float4, Hmax)   @shSharedParameter(Hmax)
	shUniform(float4, Hsm)    @shSharedParameter(Hsm)

	shUniform(float4, Amin)   @shSharedParameter(Amin)
	shUniform(float4, Amax)   @shSharedParameter(Amax)
	shUniform(float4, Asm)    @shSharedParameter(Asm)

	shUniform(float4, fNoise) @shSharedParameter(fNoise)
	#endif

	shSampler2D(samHMap)

SH_START_PROGRAM
{
	float h = shSample(samHMap, float2(uv.x,1-uv.y) );
	
	//shOutputColour(0) = float4(uv.x, uv.y, 1, 1);

	float ah = abs(h);
	shOutputColour(0) = float4(ah*0.04, ah*0.01, ah*0.004, 0/*abs(4-ah)/4*/);
}

#endif
