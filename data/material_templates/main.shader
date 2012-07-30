#include "core.h"


#define FOG @shGlobalSettingBool(fog)

#define SHADOWS @shGlobalSettingBool(shadows_pssm)

#if SHADOWS
    #include "shadows.h"
#endif

#if FOG || SHADOWS
#define NEED_DEPTH
#endif

#define ALPHA_MAP @shPropertyBool(alpha_map)
#define NORMAL_MAP @shPropertyBool(normal_map)
#define ENV_MAP @shPropertyBool(env_map)
#define FRESNEL @shPropertyBool(fresnel)
#define REFL_MAP @shPropertyBool(refl_map)


#ifdef SH_VERTEX_SHADER

    // ------------------------------------- VERTEX ---------------------------------------

    SH_BEGIN_PROGRAM
        shUniform(float4x4, wvp) @shAutoConstant(wvp, worldviewproj_matrix)
        shVertexInput(float2, uv0)
        shOutput(float2, UV)
        shNormalInput(float4)
#ifdef NEED_DEPTH
        shOutput(float, depthPassthrough)
#endif

#if NORMAL_MAP
        shTangentInput(float3)
        shOutput(float3, tangentPassthrough)
#endif

        shOutput(float3, normalPassthrough)
        shOutput(float3, objSpacePositionPassthrough)

#if SHADOWS
    @shForeach(3)
        shOutput(float4, lightSpacePos@shIterator)
        shUniform(float4x4, texWorldViewProjMatrix@shIterator) @shAutoConstant(texWorldViewProjMatrix@shIterator, texture_worldviewproj_matrix, @shIterator)
    @shEndForeach

#endif
    SH_START_PROGRAM
    {
	    shOutputPosition = shMatrixMult(wvp, shInputPosition);
	    UV = uv0;

        normalPassthrough = normal.xyz;

#ifdef NEED_DEPTH
        depthPassthrough = shOutputPosition.z;
#endif

        objSpacePositionPassthrough = shInputPosition.xyz;

#if NORMAL_MAP
        tangentPassthrough = tangent;
#endif

#if SHADOWS
    @shForeach(3)
        lightSpacePos@shIterator = shMatrixMult(texWorldViewProjMatrix@shIterator, shInputPosition);
    @shEndForeach
#endif
    }

#else

    // ----------------------------------- FRAGMENT ------------------------------------------


    SH_BEGIN_PROGRAM
		shSampler2D(diffuseMap)
		shInput(float2, UV)
		
#if ALPHA_MAP
        shSampler2D(alphaMap)
#endif

#if NORMAL_MAP
        shSampler2D(normalMap)
#endif

#if ENV_MAP
        shSamplerCube(envMap)
        
        shUniform(float3, camPosWS) @shAutoConstant(camPosWS, camera_position)
        shUniform(float4x4, wMat) @shAutoConstant(wMat, world_matrix)
        
        #if REFL_MAP
        shSampler2D(reflMap)
        #endif
        
        #if FRESNEL
        shUniform(float3, fresnelScaleBiasPower) @shUniformProperty3f(fresnelScaleBiasPower, fresnelScaleBiasPower)
        #endif
#endif

#ifdef NEED_DEPTH
        shInput(float, depthPassthrough)
#endif

#if NORMAL_MAP
        shInput(float3, tangentPassthrough)
        
        shUniform(float, bumpScale) @shUniformProperty1f(bumpScale, bump_scale)
#endif

        shInput(float3, normalPassthrough)
        shInput(float3, objSpacePositionPassthrough)
        shUniform(float4, lightAmbient)                       @shAutoConstant(lightAmbient, ambient_light_colour)
        shUniform(float4, materialAmbient)                    @shAutoConstant(materialAmbient, surface_ambient_colour)
        shUniform(float4, materialDiffuse)                    @shAutoConstant(materialDiffuse, surface_diffuse_colour)
        shUniform(float4, materialSpecular)                   @shAutoConstant(materialSpecular, surface_specular_colour)

        shUniform(float4, lightPosObjSpace)        @shAutoConstant(lightPosObjSpace, light_position_object_space)
        shUniform(float4, lightSpecular)           @shAutoConstant(lightSpecular, light_specular_colour)
        shUniform(float4, lightDiffuse)            @shAutoConstant(lightDiffuse, light_diffuse_colour)

        
        shUniform(float3, camPosObjSpace)          @shAutoConstant(camPosObjSpace, camera_position_object_space)
        
#if FOG
        shUniform(float3, fogColour) @shAutoConstant(fogColour, fog_colour)
        shUniform(float4, fogParams) @shAutoConstant(fogParams, fog_params)
#endif

#if SHADOWS
    @shForeach(3)
        shInput(float4, lightSpacePos@shIterator)
        shSampler2D(shadowMap@shIterator)
        shUniform(float2, invShadowmapSize@shIterator)  @shAutoConstant(invShadowmapSize@shIterator, inverse_texture_size, @shIterator)
    @shEndForeach
    shUniform(float3, pssmSplitPoints)  @shSharedParameter(pssmSplitPoints)
#endif

#if SHADOWS
        shUniform(float4, shadowFar_fadeStart) @shSharedParameter(shadowFar_fadeStart)
#endif


    SH_START_PROGRAM
    {
        shOutputColour(0) = shSample(diffuseMap, UV);


        float3 normal = normalize(normalPassthrough);

#if NORMAL_MAP

		float3 binormal = cross(tangentPassthrough.xyz, normal.xyz);
		float3x3 tbn = float3x3(tangentPassthrough.xyz * bumpScale, binormal * bumpScale, normal.xyz);
		
		#if SH_GLSL
		tbn = transpose(tbn);
		#endif

        float3 TSnormal = shSample(normalMap, UV).xyz * 2 - 1;
        
        normal = normalize (shMatrixMult( transpose(tbn), TSnormal ));
#endif

        float3 ambient = materialAmbient.xyz * lightAmbient.xyz;
        
        
    
    
        // shadows
#if SHADOWS
            float shadow = pssmDepthShadow (lightSpacePos0, invShadowmapSize0, shadowMap0, lightSpacePos1, invShadowmapSize1, shadowMap1, lightSpacePos2, invShadowmapSize2, shadowMap2, depthPassthrough, pssmSplitPoints);
#endif

#if SHADOWS
            float fadeRange = shadowFar_fadeStart.x - shadowFar_fadeStart.y;
            float fade = 1-((depthPassthrough - shadowFar_fadeStart.y) / fadeRange);
            shadow = (depthPassthrough > shadowFar_fadeStart.x) ? 1 : ((depthPassthrough > shadowFar_fadeStart.y) ? 1-((1-shadow)*fade) : shadow);
#endif

#if !SHADOWS
            float shadow = 1.0;
#endif


        float3 lightDir = normalize(lightPosObjSpace.xyz);

        float3 diffuse = materialDiffuse.xyz * max(dot(normal, lightDir), 0) * shadow;
    
        float3 eyeDir = normalize(camPosObjSpace.xyz - objSpacePositionPassthrough.xyz);
        float3 halfAngle = normalize (lightDir + eyeDir);
        float specular = pow(max(dot(normal, halfAngle), 0), materialSpecular.w);


        shOutputColour(0).xyz *= (ambient + diffuse + specular * materialSpecular.xyz * lightSpecular.xyz * shadow);


#if ENV_MAP           
        float3 r = reflect( -eyeDir, normal );
        r = normalize(shMatrixMult(wMat, float4(r, 0)).xyz); 
        
        r.z = -r.z;
		float4 envColor = shCubicSample(envMap, r);
		
		float reflectionFactor = 1;
		
		#if REFL_MAP
		reflectionFactor *= shSample(reflMap, UV).r;
		#endif
		
		#if FRESNEL
		
		float facing = 1.0 - max(abs(dot(-eyeDir, normal)), 0);
		reflectionFactor *= shSaturate(fresnelScaleBiasPower.y + fresnelScaleBiasPower.x * pow(facing, fresnelScaleBiasPower.z));
		

		#endif
		
		shOutputColour(0).xyz = shLerp (shOutputColour(0).xyz, envColor.xyz, reflectionFactor);
		
#endif


#if FOG
        float fogValue = shSaturate((depthPassthrough - fogParams.y) * fogParams.w);
        
        shOutputColour(0).xyz = shLerp (shOutputColour(0).xyz, fogColour, fogValue);
#endif
        
#if ALPHA_MAP
        shOutputColour(0).a = shSample(alphaMap, float2(UV.x, UV.y * 0.01)).r;
#endif


    }

#endif
