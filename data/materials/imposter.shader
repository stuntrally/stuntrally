#include "core.h"

#ifdef SH_VERTEX_SHADER

	SH_BEGIN_PROGRAM
		shUniform (float4, preRotatedQuad[4])

		shUniform(float4x4, worldViewProj)  @shAutoConstant(worldViewProj, worldviewproj_matrix)
		shVertexInput(float2, uv0)
		shNormalInput(float4)

		shOutput(float4, objSpacePosition)
		shOutput(float2, uvPassthrough)
		shOutput(float, depth)
		
		shUniform(float4x4, textureMatrix) @shAutoConstant(textureMatrix, texture_matrix, 0)

	SH_START_PROGRAM
	{
		//Face the camera
		float4 vCenter = float4( shInputPosition.x, shInputPosition.y, shInputPosition.z, 1.0f );
		float4 vScale = float4( normal.x, normal.y, normal.x, 1.0f );
		float4 pos = vCenter + (preRotatedQuad[int(normal.z)] * vScale);
		shOutputPosition = shMatrixMult( worldViewProj, pos );

		uvPassthrough = shMatrixMult(textureMatrix,float4(uv0,0,1)).xy;
		
		objSpacePosition = pos;
		depth = shOutputPosition.z;
	}

#else

	SH_BEGIN_PROGRAM
		shInput(float4, objSpacePosition)
		shInput(float2, uvPassthrough)
		shInput(float, depth)
		shSampler2D(diffuseMap)
		
	//#if FOG
        shUniform(float4, fogParams)  @shAutoConstant(fogParams, fog_params)
		shUniform(float4, fogColorSun)   @shSharedParameter(fogColorSun)
		shUniform(float4, fogColorAway)  @shSharedParameter(fogColorAway)
		shUniform(float4, fogColorH)     @shSharedParameter(fogColorH)
		shUniform(float4, fogParamsH)    @shSharedParameter(fogParamsH)

		shUniform(float4, fogFluidH)     @shSharedParameter(fogFluidH)
		shUniform(float4, fogFluidClr)   @shSharedParameter(fogFluidClr)
	//#endif
		shUniform(float4, cameraPos)  @shAutoConstant(cameraPos, camera_position_object_space)
		shUniform(float4, lightPosObjSpace)	 @shAutoConstant(lightPosObjSpace, light_position_object_space)


		shUniform(float4x4, worldMatrix)  @shAutoConstant(worldMatrix, world_matrix)

	SH_START_PROGRAM
	{
		shOutputColour(0) = shSample(diffuseMap, uvPassthrough);

		//#if FOG
		float3 lightDir = lightPosObjSpace.xyz; // directional
		float3 eyeDir = cameraPos.xyz - objSpacePosition.xyz;
        float worldPosY = shMatrixMult(worldMatrix, float4(objSpacePosition.xyz, 1)).y;

		///_ calculate fog
		float fogDepth = shSaturate((depth - fogParams.y) * fogParams.w);  // w = 1 / (max - min)
		float fogDepthH = shSaturate((depth - fogParamsH.z) * fogParamsH.w);

		float fogDir = dot( normalize(eyeDir.xz), normalize(lightDir.xz) ) * 0.5 + 0.5;
		float fogH = shSaturate( (fogParamsH.x/*h*/ - worldPosY) * fogParamsH.y/*dens*/);

		float4 fogClrDir = shLerp( fogColorAway, fogColorSun, fogDir);
		float4 fogClrFinal = shLerp( fogClrDir, fogColorH, fogH);
		float fogL = shLerp( fogDepth * fogClrDir.a, fogDepthH * fogColorH.a, fogH);

        /// fluid fog
        float flDepth = shSaturate(depth * fogFluidH.y);
        float flH = shSaturate((fogFluidH.x/*h*/ - worldPosY) * fogFluidH.z);
        float4 flClrFinal = shLerp( fogClrFinal, fogFluidClr, flH);
		float flL = shLerp( fogL, flDepth /* * fogFluidClr.a*/, flH);

        shOutputColour(0).xyz = shLerp( shOutputColour(0).xyz, flClrFinal.rgb, flL);
		///_

		//#endif
	}

#endif
