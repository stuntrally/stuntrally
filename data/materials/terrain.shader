#include "core.h"



#define RENDER_COMPOSITE_MAP  @shPropertyBool(composite_map)

#define COMPOSITE_MAP  @shGlobalSettingBool(terrain_composite_map)

#define FOG  @shGlobalSettingBool(fog) && !RENDER_COMPOSITE_MAP
#define MRT  (!RENDER_COMPOSITE_MAP && @shGlobalSettingBool(mrt_output))

#define SHADOWS        @shGlobalSettingBool(shadows_pssm) && !RENDER_COMPOSITE_MAP
#define SHADOWS_DEPTH  @shGlobalSettingBool(shadows_depth)

#if SHADOWS
#include "shadows.h"
#endif

#define NUM_LAYERS  @shPropertyString(num_layers)

#define DEBUG_BLEND  @shGlobalSettingBool(debug_blend)

#define NORMAL_MAPPING  @shGlobalSettingBool(terrain_normal)

#define SPECULAR  @shGlobalSettingBool(terrain_specular) && !RENDER_COMPOSITE_MAP

#define EMISSIVE_SPECULAR  @shGlobalSettingBool(terrain_emissive_specular)


//  parallax
#define PARALLAX_MAPPING  @shGlobalSettingBool(terrain_parallax) && !RENDER_COMPOSITE_MAP && NORMAL_MAPPING

#define PARALLAX_SCALE  0.03
#define PARALLAX_BIAS  -0.04


///  triplanar
#define TRIPLANAR_TYPE  @shGlobalSettingString(terrain_triplanarType)
#define TRIPLANAR_FULL  (TRIPLANAR_TYPE == 2)
#define TRIPLANAR_1  (TRIPLANAR_TYPE == 1)
#define TRIPLANAR    (TRIPLANAR_TYPE) && !RENDER_COMPOSITE_MAP
//  1 layer only
#define TRIPLANAR_LAYER  @shGlobalSettingString(terrain_triplanarLayer)
//  2 layers only
#define TRIPLANAR_LAYER2  @shGlobalSettingString(terrain_triplanarLayer2)


#if (MRT) || (FOG) || (SHADOWS)
#define NEED_DEPTH 1
#endif


#if NEED_DEPTH
@shAllocatePassthrough(1, depth)
#endif

@shAllocatePassthrough(2, UV)

@shAllocatePassthrough(4, objSpacePosition)

#if MRT
@shAllocatePassthrough(3, viewPosition)
#endif

#if SHADOWS
@shForeach(3)
    @shAllocatePassthrough(4, lightSpacePos@shIterator)
@shEndForeach
#endif

#ifdef SH_VERTEX_SHADER

    // ------------------------------------- VERTEX ---------------------------------------

    SH_BEGIN_PROGRAM
        shUniform(float4x4, worldMatrix)  @shAutoConstant(worldMatrix, world_matrix)
        shUniform(float4x4, viewProjMatrix)  @shAutoConstant(viewProjMatrix, viewproj_matrix)
        
        shUniform(float2, lodMorph)  @shAutoConstant(lodMorph, custom, 1001)
        
        shVertexInput(float2, uv0)
        shVertexInput(float2, uv1) // lodDelta, lodThreshold
        
#if MRT
        shUniform(float4x4, wvMat) @shAutoConstant(wvMat, worldview_matrix)
#endif

#if SHADOWS
    @shForeach(3)
        shUniform(float4x4, texViewProjMatrix@shIterator) @shAutoConstant(texViewProjMatrix@shIterator, texture_viewproj_matrix, @shIterator)
    @shEndForeach
#endif

        
        @shPassthroughVertexOutputs

    SH_START_PROGRAM
    {

#if MRT
        float3 viewPos = shMatrixMult(wvMat, shInputPosition).xyz;
        @shPassthroughAssign(viewPosition, viewPos);
#endif


        float4 worldPos = shMatrixMult(worldMatrix, shInputPosition);

        // determine whether to apply the LOD morph to this vertex
        // we store the deltas against all vertices so we only want to apply 
        // the morph to the ones which would disappear. The target LOD which is
        // being morphed to is stored in lodMorph.y, and the LOD at which 
        // the vertex should be morphed is stored in uv.w. If we subtract
        // the former from the latter, and arrange to only morph if the
        // result is negative (it will only be -1 in fact, since after that
        // the vertex will never be indexed), we will achieve our aim.
        // sign(vertexLOD - targetLOD) == -1 is to morph
        float toMorph = -min(0, sign(uv1.y - lodMorph.y));

        // morph
        // this assumes XZ terrain alignment
        worldPos.y += uv1.x * toMorph * lodMorph.x;


        shOutputPosition = shMatrixMult(viewProjMatrix, worldPos);
        
#if NEED_DEPTH
        @shPassthroughAssign(depth, shOutputPosition.z);
#endif

        @shPassthroughAssign(UV, uv0);
        
        @shPassthroughAssign(objSpacePosition, shInputPosition);


#if SHADOWS
        float4 wPos = shMatrixMult(worldMatrix, shInputPosition);
        
        float4 lightSpacePos;
    @shForeach(3)
        lightSpacePos = shMatrixMult(texViewProjMatrix@shIterator, wPos);
        @shPassthroughAssign(lightSpacePos@shIterator, lightSpacePos);
    @shEndForeach
#endif

    }

#else

    // ----------------------------------- FRAGMENT ------------------------------------------


#if !COMPOSITE_MAP



    SH_BEGIN_PROGRAM

        shSampler2D(normalMap) // global normal map

        shSampler2D(lightMap)

    @shForeach(@shPropertyString(num_blendmaps))
        shSampler2D(blendMap@shIterator)
    @shEndForeach

    @shForeach(@shPropertyString(num_layers))
        shSampler2D(diffuseMap@shIterator)
    @shEndForeach

    @shForeach(@shPropertyString(num_layers))
        shSampler2D(normalMap@shIterator)
    @shEndForeach
    
#if TRIPLANAR || FOG
        shUniform(float4x4, worldMatrix)  @shAutoConstant(worldMatrix, world_matrix)
#endif
#if TRIPLANAR
        shUniform(float, terrainWorldSize)  @shSharedParameter(terrainWorldSize)
#endif

#if NORMAL_MAPPING
        shUniform(float, ter_scaleNormal)   @shSharedParameter(ter_scaleNormal)
#endif
        shUniform(float, ter_specular_pow)     @shSharedParameter(ter_specular_pow)
        shUniform(float, ter_specular_pow_em)  @shSharedParameter(ter_specular_pow_em)

        // layer uv multipliers
    @shForeach(@shPropertyString(num_uv_mul))
        shUniform(float4, uvMul@shIterator)  @shUniformProperty4f(uvMul@shIterator, uv_mul_@shIterator)
    @shEndForeach
    
#if FOG
        shUniform(float4, fogParams)  @shAutoConstant(fogParams, fog_params)
		shUniform(float4, fogColorSun)   @shSharedParameter(fogColorSun)
		shUniform(float4, fogColorAway)  @shSharedParameter(fogColorAway)
		shUniform(float4, fogColorH)     @shSharedParameter(fogColorH)
		shUniform(float4, fogParamsH)    @shSharedParameter(fogParamsH)

		shUniform(float4, fogFluidH)     @shSharedParameter(fogFluidH)
		shUniform(float4, fogFluidClr)   @shSharedParameter(fogFluidClr)
#endif
    
        @shPassthroughFragmentInputs
    
#if MRT
        shDeclareMrtOutput(1)
        shDeclareMrtOutput(2)
        shUniform(float, far)  @shAutoConstant(far, far_clip_distance)
        shUniform(float4x4, wvMat)  @shAutoConstant(wvMat, worldview_matrix)
#endif


        shUniform(float4, lightAmbient)                  @shAutoConstant(lightAmbient, ambient_light_colour)
    @shForeach(1)
        shUniform(float4, lightPosObjSpace@shIterator)   @shAutoConstant(lightPosObjSpace@shIterator, light_position_object_space, @shIterator)
        shUniform(float4, lightSpecular@shIterator)      @shAutoConstant(lightSpecular@shIterator, light_specular_colour, @shIterator)
        shUniform(float4, lightDiffuse@shIterator)       @shAutoConstant(lightDiffuse@shIterator, light_diffuse_colour, @shIterator)
    @shEndForeach
    
        shUniform(float3, eyePosObjSpace)                @shAutoConstant(eyePosObjSpace, camera_position_object_space)



#if SHADOWS
    @shForeach(3)
        shSampler2D(shadowMap@shIterator)
        shUniform(float2, invShadowmapSize@shIterator)  @shAutoConstant(invShadowmapSize@shIterator, inverse_texture_size, @shIterator)
    @shEndForeach
    shUniform(float3, pssmSplitPoints)  @shSharedParameter(pssmSplitPoints)
#endif

#if SHADOWS
        shUniform(float4, shadowFar_fadeStart)  @shSharedParameter(shadowFar_fadeStart)
#endif


    SH_START_PROGRAM
    {

#if NEED_DEPTH
        float depth = @shPassthroughReceive(depth);
#endif

        float2 UV = @shPassthroughReceive(UV);
        
        float4 objSpacePosition = @shPassthroughReceive(objSpacePosition);


        // Shadows
#if SHADOWS
        @shForeach(3)
            float4 lightSpacePos@shIterator = @shPassthroughReceive(lightSpacePos@shIterator);
        @shEndForeach

            float shadow = pssmDepthShadow (lightSpacePos0, invShadowmapSize0, shadowMap0, lightSpacePos1, invShadowmapSize1, shadowMap1, lightSpacePos2, invShadowmapSize2, shadowMap2, depth, pssmSplitPoints, 0.f);

            float fadeRange = shadowFar_fadeStart.x - shadowFar_fadeStart.y;
            float fade = 1-((depth - shadowFar_fadeStart.y) / fadeRange);
            shadow = (depth > shadowFar_fadeStart.x) ? 1.0 : ((depth > shadowFar_fadeStart.y) ? 1.0-((1.0-shadow)*fade) : shadow);
#endif



#if !(SHADOWS)
            float shadow = 1.0;
#endif

		shadow *= shSample(lightMap, UV).x;
        



        float3 normal = shSample(normalMap, UV).rgb * 2 - 1;
        normal = normalize(normal);
        
        
        // derive the tangent space basis
        float3 tangent = float3(1, 0, 0); // XZ terrain align
        
        float3 binormal = normalize(cross(tangent, normal));
        tangent = normalize(cross(normal, binormal)); // note, now we need to re-cross to derive tangent again because it wasn't orthonormal

	    // derive final matrix
		float3x3 TBN = float3x3(tangent, binormal, normal);
		
		#if SH_GLSL
		TBN = transpose(TBN);
		#endif
		
		float3 lightDir = lightPosObjSpace0.xyz; // directional
		float3 eyeDir = eyePosObjSpace.xyz - objSpacePosition.xyz;

#if NORMAL_MAPPING
		float3 TSlightDir = normalize(shMatrixMult(TBN, lightDir));
		float3 TSeyeDir = normalize(shMatrixMult(TBN, eyeDir));
        float3 TShalfAngle = normalize(TSlightDir + TSeyeDir);
#endif
        
        // set up blend values
@shForeach(@shPropertyString(num_blendmaps))
        float4 blendValues@shIterator = shSample(blendMap@shIterator, UV);
@shEndForeach
        
        
        
#if TRIPLANAR
        // Determine the blend weights for the 3 planar projections.  
        float3 absNormal = abs( normal.xyz );   
        float3 blend_weights = absNormal;
        // Tighten up the blending zone:  

        // -> ORIGINAL : 
        //blend_weights = (blend_weights - 0.2) * 7; // => in original paper,
        // 	but I don't see how the *7 is supposed to change anything, 
        //	since there is a division by itself just after (see "Force weights to sum to 1.0").
        blend_weights = (blend_weights - 0.5);
        blend_weights = max(blend_weights, 0.0001);

        // Force weights to sum to 1.0
        blend_weights /= blend_weights.x + blend_weights.y + blend_weights.z;
        
        float2 coord1, coord2, coord3;
        float4 col1, col2, col3 = float4(1,1,1,1);
        
        // use world position and divide by terrain world size to get a consistent uv scale (-0.5 ... 0.5)
        // this is the same scale that non-triplanar would have
        float3 wPos = shMatrixMult(worldMatrix, float4(objSpacePosition.xyz, 1)).xyz / terrainWorldSize;
#endif
        
#if NORMAL_MAPPING
        // normal mapping - per-layer lighting
        float3 TSnormal;
        float2 litRes = float2(0,0); // diffuse, specular amount
        float NdotL;
        float specular;
#else
        float specularAmount = 0;
#endif
        
        //  vars
        float3 albedo = float3(0,0,0);
        float3 bb;
        float4 diffuseSpec;
        float uvMul;
		float fBlend;

        
//-----  per layer calculations
    @shForeach(@shPropertyString(num_layers))
    
        fBlend = blendValues@shPropertyString(blendmap_component_@shIterator);
    
///---------------------------------------------------------------------------------------------
#if TRIPLANAR

	#if (TRIPLANAR_FULL) || (TRIPLANAR_LAYER == @shIterator) || (TRIPLANAR_LAYER2 == @shIterator)
		/// triplanar on all  or on this layer

		coord1 = wPos.yz * uvMul@shPropertyString(uv_component_@shIterator);
		coord2 = wPos.zx * uvMul@shPropertyString(uv_component_@shIterator);
		coord3 = wPos.xy * uvMul@shPropertyString(uv_component_@shIterator);
		coord3.x *= -1.f;
		
        // parallax
        #if PARALLAX_MAPPING
		if (blend_weights.x > 0)  coord1 += TSeyeDir.xy * ( shSample(normalMap@shIterator, coord1).a * PARALLAX_SCALE + PARALLAX_BIAS );
		if (blend_weights.y > 0)  coord2 += TSeyeDir.xy * ( shSample(normalMap@shIterator, coord2).a * PARALLAX_SCALE + PARALLAX_BIAS );
		if (blend_weights.z > 0)  coord3 += TSeyeDir.xy * ( shSample(normalMap@shIterator, coord3).a * PARALLAX_SCALE + PARALLAX_BIAS );
        #endif

		// Sample color maps for each projection, at those UV coords.
									col1 = shSample(diffuseMap@shIterator, coord1.yx);
		if (blend_weights.y > 0)	col2 = shSample(diffuseMap@shIterator, coord2.yx);
		if (blend_weights.z > 0)	col3 = shSample(diffuseMap@shIterator, coord3);

		// Finally, blend the results of the 3 planar projections.
		diffuseSpec = col1.xyzw * blend_weights.xxxx +  col2.xyzw * blend_weights.yyyy +  col3.xyzw * blend_weights.zzzz; 

        // normal
        #if NORMAL_MAPPING
									col1 = shSample(normalMap@shIterator, coord1.yx) * 2 - 1;
		if (blend_weights.y > 0)	col2 = shSample(normalMap@shIterator, coord2.yx) * 2 - 1;
		if (blend_weights.z > 0)	col3 = shSample(normalMap@shIterator, coord3) * 2 - 1;
		TSnormal = normalize(col1.xyz * blend_weights.xxx +  col2.xyz * blend_weights.yyy +  col3.xyz * blend_weights.zzz);
        #endif
	#else
		/// no triplanar on layer
        uvMul = uvMul@shPropertyString(uv_component_@shIterator);

        // parallax
        #if PARALLAX_MAPPING
        float2 layerUV@shIterator = UV * uvMul + TSeyeDir.xy * ( shSample(normalMap@shIterator, UV * uvMul).a * PARALLAX_SCALE + PARALLAX_BIAS );
        #else
        float2 layerUV@shIterator = UV * uvMul;
        #endif
        
        diffuseSpec = shSample(diffuseMap@shIterator, layerUV@shIterator);
            
        // normal
        #if NORMAL_MAPPING
        TSnormal = normalize(shSample(normalMap@shIterator, layerUV@shIterator).xyz * 2 - 1);
        #endif
	#endif

#else	///  no triplanar
        uvMul = uvMul@shPropertyString(uv_component_@shIterator);

        // parallax
        #if PARALLAX_MAPPING
        float2 layerUV@shIterator = UV * uvMul + TSeyeDir.xy * ( shSample(normalMap@shIterator, UV * uvMul).a * PARALLAX_SCALE + PARALLAX_BIAS );
        #else
        float2 layerUV@shIterator = UV * uvMul;
        #endif
        
        diffuseSpec = shSample(diffuseMap@shIterator, layerUV@shIterator);
            
        // normal
        #if NORMAL_MAPPING
        TSnormal = normalize(shSample(normalMap@shIterator, layerUV@shIterator).xyz * 2 - 1);
        #endif
#endif
///---------------------------------------------------------------------------------------------

        
        ////  albedo

	#if DEBUG_BLEND
		//  for test
        bb = float3(0,0,0);
        #if @shIterator == 0
        bb = float3(1,0,0);
        #endif
        #if @shIterator == 1
        bb = float3(0,1,0);
        #endif
        #if @shIterator == 2
        bb = float3(0,0,1);
        #endif
        #if @shIterator == 3  // only 4
        bb = float3(0.5,0.5,0.5);
        #endif
        albedo += bb * fBlend;
    #else
        albedo += diffuseSpec.rgb * fBlend;
    #endif

	#if NORMAL_MAPPING
		TSnormal.z *= ter_scaleNormal;
		TSnormal = normalize(TSnormal);
		
        NdotL = max(dot(TSnormal, TSlightDir), 0);
        #if EMISSIVE_SPECULAR
			specular = pow(diffuseSpec.a, ter_specular_pow_em);
        #else
			specular = pow(max(dot(TSnormal, TShalfAngle), 0), ter_specular_pow) * diffuseSpec.a;
		#endif
        
        #if @shIterator == 0
	        litRes.x = NdotL;
		    #if SPECULAR
			litRes.y = specular;
			#endif
        #else
			litRes.x = shLerp (litRes.x, NdotL, fBlend);
			#if SPECULAR
			litRes.y = shLerp (litRes.y, specular, fBlend);
			#endif
        #endif
        
	#else
        
        #if @shIterator == 0
        specularAmount = diffuseSpec.a;
        #else
        specularAmount = shLerp(specularAmount, diffuseSpec.a, fBlend);
        #endif
        
	#endif
        
        
    @shEndForeach
//-----  per layer
	
        
        shOutputColour(0).a = 1.f;
        shOutputColour(0).rgb = albedo;
        
        
        // Lighting

#if !NORMAL_MAPPING
        // no normal mapping - light all layers at once
       
        float3 diffuse = float3(0,0,0);
       
        lightDir = normalize(lightDir);
        float3 halfAngle = normalize(lightDir + eyeDir);
        
        float specular = pow(max(dot(normal, halfAngle), 0), ter_specular_pow);

        diffuse += lightDiffuse0.xyz * max(dot(normal, lightDir), 0) * shadow;
    
	#if DEBUG_BLEND
        shOutputColour(0).xyz *= (float3(0.5,0.5,0.5) + 0.5*diffuse);
    #else
        shOutputColour(0).xyz *= (lightAmbient.xyz + diffuse);
    #endif
        #if SPECULAR
			#if EMISSIVE_SPECULAR
        		shOutputColour(0).xyz +=  specular * lightSpecular0.xyz * specularAmount;// * diffuse;
			#else
        		shOutputColour(0).xyz +=  specular * lightSpecular0.xyz * specularAmount * shadow;
	        #endif
        #endif
#else
	#if DEBUG_BLEND
        shOutputColour(0).xyz *= (float3(0.5,0.5,0.5) + litRes.x * float3(0.8,0.8,0.8) * shadow);
    #else
        shOutputColour(0).xyz *= (lightAmbient.xyz + litRes.x * lightDiffuse0.xyz * shadow);
    #endif
        #if SPECULAR
			#if EMISSIVE_SPECULAR
		        shOutputColour(0).xyz +=  litRes.y * lightSpecular0.xyz;// * litRes.x;
			#else
		        shOutputColour(0).xyz +=  litRes.y * lightSpecular0.xyz * shadow;
	        #endif
        #endif
#endif
        
        
        
#if FOG
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
#endif

#if MRT
        float3 viewPosition = @shPassthroughReceive(viewPosition);
        float3 viewNormal = normalize(shMatrixMult(wvMat, float4(normal, 0)).xyz);
        shOutputColour(1) = float4(length(viewPosition) / far, normalize(viewNormal));
        shOutputColour(2) = float4(depth / far, 0, depth / objSpacePosition.w, 1.0); // .w motionblur mask
#endif


    }
    
    
    
    
    
    
#else // COMPOSITE_MAP  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -






    SH_BEGIN_PROGRAM
    
#if FOG
        shUniform(float4, fogParams)  @shAutoConstant(fogParams, fog_params)
		shUniform(float4, fogColorSun)   @shSharedParameter(fogColorSun)
		shUniform(float4, fogColorAway)  @shSharedParameter(fogColorAway)
		shUniform(float4, fogColorH)     @shSharedParameter(fogColorH)
		shUniform(float4, fogParamsH)    @shSharedParameter(fogParamsH)

		shUniform(float4, fogFluidH)     @shSharedParameter(fogFluidH)
		shUniform(float4, fogFluidClr)   @shSharedParameter(fogFluidClr)

        shUniform(float3, eyePosObjSpace)    @shAutoConstant(eyePosObjSpace, camera_position_object_space)
		shUniform(float4, lightPosObjSpace)	 @shAutoConstant(lightPosObjSpace, light_position_object_space)
		shUniform(float4x4, worldMatrix)  @shAutoConstant(worldMatrix, world_matrix)
#endif
    
        @shPassthroughFragmentInputs
    
#if MRT
        shDeclareMrtOutput(1)
        shDeclareMrtOutput(2)
        shUniform(float, far) @shAutoConstant(far, far_clip_distance)
        shUniform(float4x4, wvMat) @shAutoConstant(wvMat, worldview_matrix)
#endif



#if SHADOWS
    @shForeach(3)
        shSampler2D(shadowMap@shIterator)
        shUniform(float2, invShadowmapSize@shIterator)  @shAutoConstant(invShadowmapSize@shIterator, inverse_texture_size, @shIterator)
    @shEndForeach
    shUniform(float3, pssmSplitPoints)  @shSharedParameter(pssmSplitPoints)
#endif

#if SHADOWS
        shUniform(float4, shadowFar_fadeStart)  @shSharedParameter(shadowFar_fadeStart)
#endif


        shSampler2D(compositeMap)

#if MRT
	shSampler2D(normalMap)
#endif


    SH_START_PROGRAM
    {

#if NEED_DEPTH
        float depth = @shPassthroughReceive(depth);
#endif

        float2 UV = @shPassthroughReceive(UV);
        
        float4 objSpacePosition = @shPassthroughReceive(objSpacePosition);


        // Shadows
#if SHADOWS
        @shForeach(3)
            float4 lightSpacePos@shIterator = @shPassthroughReceive(lightSpacePos@shIterator);
        @shEndForeach

            float shadow = pssmDepthShadow (lightSpacePos0, invShadowmapSize0, shadowMap0, lightSpacePos1, invShadowmapSize1, shadowMap1, lightSpacePos2, invShadowmapSize2, shadowMap2, depth, pssmSplitPoints, 0.f);

            float fadeRange = shadowFar_fadeStart.x - shadowFar_fadeStart.y;
            float fade = 1-((depth - shadowFar_fadeStart.y) / fadeRange);
            shadow = (depth > shadowFar_fadeStart.x) ? 1.0 : ((depth > shadowFar_fadeStart.y) ? 1.0-((1.0-shadow)*fade) : shadow);
#endif



#if !(SHADOWS)
            float shadow = 1.0;
#endif
        
        
        shOutputColour(0) = float4(shSample(compositeMap, UV).xyz, 1);

        
#if FOG  
		float3 lightDir = lightPosObjSpace.xyz; // directional
		float3 eyeDir = eyePosObjSpace.xyz - objSpacePosition.xyz;
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
#endif


#if MRT
        float3 normal = shSample(normalMap, UV).rgb * 2 - 1;
        normal = normalize(normal);

        float3 viewPosition = @shPassthroughReceive(viewPosition);
        float3 viewNormal = normalize(shMatrixMult(wvMat, float4(normal, 0)).xyz);
        shOutputColour(1) = float4(length(viewPosition) / far, normalize(viewNormal));
        shOutputColour(2) = float4(depth / far, 0, depth / objSpacePosition.w, 1.0); // .w motionblur mask
#endif

		//shOutputColour(0).xy = UV;

    }




#endif
    

#endif
