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

	//shUniform(float3, arrowColour1)     @shSharedParameter(arrowColour1)

	shSampler2D(HMap)

SH_START_PROGRAM
{
	float h = shSample(HMap, float2(uv.x,1-uv.y) );
	
	//shOutputColour(0).xyz = float3(uv.x, uv.y, 1);
	shOutputColour(0).xyz = float3(abs(h)*0.04, abs(h)*0.01, abs(h)*0.004);

	shOutputColour(0).w = 1;
}

#endif
