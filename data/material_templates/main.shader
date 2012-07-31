#include "core.h"


#define FOG @shGlobalSettingBool(fog)

#define SHADOWS @shGlobalSettingBool(shadows_pssm) && @shPropertyBool(receives_shadows)

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
#define CAR_PAINT_MAP @shPropertyBool(car_paint_map)
#define TERRAIN_LIGHT_MAP @shPropertyBool(terrain_light_map)


#if TERRAIN_LIGHT_MAP || ENV_MAP
#define NEED_WORLD_MATRIX
#endif

#define SPECULAR_ALPHA @shPropertyBool(specular_alpha)

#define TREE_WIND @shPropertyBool(tree_wind) && @shGlobalSettingBool(wind)


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

#if TREE_WIND

        shUniform(float, windTimer) @shSharedParameter(windTimer)
        shVertexInput(float4, uv1) // windParams
        shVertexInput(float4, uv2) // originPos

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
    
        float4 position = shInputPosition;
#if TREE_WIND

/*
		float radiusCoeff = windParams.x;
		float heightCoeff = windParams.y;
		float factorX = windParams.z;
		float factorY = windParams.w;
*/
        
		position.y += sin(windTimer + uv2.z + position.y + position.x) * uv1.x * uv1.x * uv1.w;
		position.x += sin(windTimer + uv2.z ) * uv1.y * uv1.y * uv1.z;
		
#endif
	    shOutputPosition = shMatrixMult(wvp, position);
    
	    UV = uv0;

        normalPassthrough = normal.xyz;

#ifdef NEED_DEPTH
        depthPassthrough = shOutputPosition.z;
#endif

        objSpacePositionPassthrough = position.xyz;

#if NORMAL_MAP
        tangentPassthrough = tangent;
#endif


#if SHADOWS
    @shForeach(3)
        lightSpacePos@shIterator = shMatrixMult(texWorldViewProjMatrix@shIterator, position);
    @shEndForeach
#endif
    }

#else

    // ----------------------------------- FRAGMENT ------------------------------------------


    SH_BEGIN_PROGRAM
		shSampler2D(diffuseMap)
		shInput(float2, UV)
		
#ifdef NEED_WORLD_MATRIX
        shUniform(float4x4, wMat) @shAutoConstant(wMat, world_matrix)
#endif
		
		
#if CAR_PAINT_MAP
        shSampler2D(carPaintMap)
        shUniform(float3, carColour)
#endif
		
#if ALPHA_MAP
        shSampler2D(alphaMap)
#endif

#if NORMAL_MAP
        shSampler2D(normalMap)
#endif

#if ENV_MAP
        shSamplerCube(envMap)
        
        shUniform(float3, camPosWS) @shAutoConstant(camPosWS, camera_position)
        
        #if REFL_MAP
        shSampler2D(reflMap)
        #endif
        
        #if FRESNEL
        shUniform(float3, fresnelScaleBiasPower) @shUniformProperty3f(fresnelScaleBiasPower, fresnelScaleBiasPower)
        #else
        shUniform(float, reflAmount)    @shUniformProperty1f(reflAmount, refl_amount)
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
        shUniform(float, materialShininess)                   @shAutoConstant(materialShininess, surface_shininess)
        
        shUniform(float4, lightPosObjSpace)        @shAutoConstant(lightPosObjSpace, light_position_object_space)
        shUniform(float4, lightSpecular)           @shAutoConstant(lightSpecular, light_specular_colour)
        shUniform(float4, lightDiffuse)            @shAutoConstant(lightDiffuse, light_diffuse_colour)

        
        shUniform(float3, camPosObjSpace)          @shAutoConstant(camPosObjSpace, camera_position_object_space)
        
#if FOG
        shUniform(float3, fogColour) @shAutoConstant(fogColour, fog_colour)
        shUniform(float4, fogParams) @shAutoConstant(fogParams, fog_params)
#endif


#if TERRAIN_LIGHT_MAP
        shSampler2D(terrainLightMap)
        shUniform(float, terrainWorldSize)  @shSharedParameter(terrainWorldSize)
        shUniform(float, enableTerrainLightMap)
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
        
#if CAR_PAINT_MAP
        shOutputColour(0).xyz = shLerp ( shOutputColour(0).xyz, shSample(carPaintMap, UV).r * carColour, 1 - shOutputColour(0).a);
#endif


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

#if !(SHADOWS)
            float shadow = 1.0;
#endif


#if TERRAIN_LIGHT_MAP
		float shadowingLM;
		float2 worldPos = shMatrixMult(wMat, float4(objSpacePositionPassthrough, 1)).xz;
		float2 lmTexCoord = (worldPos / terrainWorldSize) + 0.5;
		shadowingLM = shSample(terrainLightMap, lmTexCoord).x;
		shadow = min(shadow, shadowingLM);
#endif


        float3 lightDir = normalize(lightPosObjSpace.xyz);

        float NdotL = max(dot(normal, lightDir), 0);
        float3 diffuse = materialDiffuse.xyz * NdotL * shadow;
    
        float3 eyeDir = normalize(camPosObjSpace.xyz - objSpacePositionPassthrough.xyz);
        float3 halfAngle = normalize (lightDir + eyeDir);
        float specular = pow(max(dot(normal, halfAngle), 0), materialShininess);
        if (NdotL <= 0)
            specular = 0;

        shOutputColour(0).xyz *= (ambient + diffuse);
        shOutputColour(0).xyz += specular * materialSpecular.xyz * lightSpecular.xyz * shadow;

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
		#else
		reflectionFactor *= reflAmount;
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

#if SPECULAR_ALPHA
        // bump alpha with specular
        shOutputColour(0).a = min(shOutputColour(0).a + specular ,1);
#endif

//shOutputColour(0).xyz = shSample(carPaintMap, UV).rgb;

    }

#endif
