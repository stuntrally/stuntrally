#include "core.h"

#ifdef SH_VERTEX_SHADER

	SH_BEGIN_PROGRAM
		shUniform(float4x4 wvp) @shAutoConstant(wvp, worldviewproj_matrix)
		shInput(float2, uv0)
		shOutput(float2, UV)
	SH_START_PROGRAM
	{
		shOutputPosition = shMatrixMult(wvp, shInputPosition);
		UV = uv0;
	}

#else

	SH_BEGIN_PROGRAM
		shSampler2D(diffuse)
		shInput(float2, UV)
		shUniform(float3 col) @shUniformProperty3f(col, color_multiplier)
		shUniform(float4 globalColorMultiplier) @shSharedParameter(globalColorMultiplier)
	SH_START_PROGRAM
	{
		shOutputColor(0) = shSample(diffuse, UV) * float4(col, 1.0) * float4(globalColorMultiplier);
	}

#endif
