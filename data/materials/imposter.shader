#include "core.h"

#ifdef SH_VERTEX_SHADER

	SH_BEGIN_PROGRAM
		shUniform(float, uScroll)
		shUniform(float, vScroll)
		shUniform (float4, preRotatedQuad[4])

		shUniform(float4x4, worldViewProj)  @shAutoConstant(worldViewProj, worldviewproj_matrix)
		shVertexInput(float2, uv0)
		shNormalInput(float4)
		shOutput(float2, uvPassthrough)

	SH_START_PROGRAM
	{
		//Face the camera
		float4 vCenter = float4( shInputPosition.x, shInputPosition.y, shInputPosition.z, 1.0f );
		float4 vScale = float4( normal.x, normal.y, normal.x, 1.0f );
		shOutputPosition = shMatrixMult( worldViewProj, vCenter + (preRotatedQuad[int(normal.z)] * vScale) );

		uvPassthrough = uv0;
		uvPassthrough.x += uScroll;
		uvPassthrough.y += vScroll;
		
	}

#else

	SH_BEGIN_PROGRAM
		shInput(float2, uvPassthrough)
		shSampler2D(diffuseMap)

		shUniform(float4x4, textureMatrix) @shAutoConstant(textureMatrix, texture_matrix, 0)

	SH_START_PROGRAM
	{
		shOutputColour(0) = shSample(diffuseMap, shMatrixMult(textureMatrix,float4(uvPassthrough,0,1)));
	}

#endif


