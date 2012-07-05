#include "core.h"

@shAllocatePassthrough(2, UV)

#ifdef SH_VERTEX_SHADER

	SH_BEGIN_PROGRAM
		shUniform(float4x4 wvp) @shAutoConstant(wvp, worldviewproj_matrix)
		shInput(float2, uv0)
		@shPassthroughVertexOutputs
	SH_START_PROGRAM
	{
		shOutputPosition = shMatrixMult(wvp, shInputPosition);
		@shPassthroughAssign(UV, uv0);
	}

#else

	SH_BEGIN_PROGRAM
		shSampler2D(diffuse)
		shUniform(float3 col) @shUniformProperty3f(col, color_multiplier)
		shUniform(float4 globalColorMultiplier) @shSharedParameter(globalColorMultiplier)
		@shPassthroughFragmentInputs
	SH_START_PROGRAM
	{
	    float2 UV = @shPassthroughReceive(UV);
		shOutputColor(0) = shSample(diffuse, UV) * float4(col, 1.0) * float4(globalColorMultiplier);
	}

#endif
