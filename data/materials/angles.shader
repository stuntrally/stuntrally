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

	shUniform(float, InvTerSize)  @shUniformProperty1f(InvTerSize, InvTerSize)
	shUniform(float, TriSize)     @shUniformProperty1f(TriSize, TriSize)

	shSampler2D(samHMap)

SH_START_PROGRAM
{
	float s = InvTerSize;  // close tex points/vertices
	float yx = shSample(samHMap, float2(uv.x + s, 1-uv.y    )) - shSample(samHMap, float2(uv.x - s, 1-uv.y    ));
	float yz = shSample(samHMap, float2(uv.x,     1-uv.y + s)) - shSample(samHMap, float2(uv.x,     1-uv.y - s));

	float3 vx = float3(TriSize, yx, 0);  // x+1 - x-1
	float3 vz = float3(0, yz, TriSize);  // z+1 - z-1
	float3 norm = normalize(-cross(vx, vz));
	float a = acos(norm.y) * 180 / 3.1415926536;  // get angle in degrees

	shOutputColour(0) = a;
}

#endif
