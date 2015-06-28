#include "core.h"


#define FOG_G  @shGlobalSettingBool(fog)
#define FOG  FOG_G &&  @shPropertyBool(fog_en)

#define SHADOWS  @shGlobalSettingBool(shadows_pssm) &&  @shPropertyBool(receives_shadows)
#define SHADOWS_DEPTH  @shGlobalSettingBool(shadows_depth)

#define MRT  @shPropertyBool(mrt_output) &&  @shGlobalSettingBool(mrt_output)

#if SHADOWS
#include "shadows.h"
#endif

#if FOG_G || (SHADOWS) || MRT
#define NEED_DEPTH
#endif

#define ALPHA_MAP  @shPropertyHasValue(alphaMap)
#define NORMAL_MAP  @shPropertyHasValue(normalMap)
#define ENV_MAP  @shPropertyBool(env_map)
#define FRESNEL  @shPropertyBool(fresnel)
#define REFL_MAP  @shPropertyHasValue(reflMap)
#define SPEC_MAP  @shPropertyHasValue(specMap)
#define CAR_PAINT_MAP  @shPropertyHasValue(carPaintMap)

#define TERRAIN_LIGHT_MAP  @shPropertyBool(terrain_light_map)
#define TERRAIN_LIGHT_MAP_TOGGLEABLE  @shPropertyBool(terrain_light_map_toggleable)
#define MOTIONBLUR_MASK @shPropertyBool(motionblur_mask)

#define INSTANCING  @shPropertyBool(instancing)
#define SOFT_PARTICLES  (@shPropertyBool(soft_particles) &&  @shGlobalSettingBool(soft_particles))
#define SELECTED_GLOW  @shGlobalSettingBool(editor)
#define SPECULAR_ALPHA  @shPropertyBool(specular_alpha)
#define SPECMAP_RGB  @shPropertyBool(specMap_rgb)

#if (TERRAIN_LIGHT_MAP) || (ENV_MAP) || (SOFT_PARTICLES) || (FOG_G)
#define NEED_WORLD_MATRIX
#endif

#define TREE_WIND  @shPropertyBool(tree_wind)
#define GRASS_WIND  @shPropertyBool(grass_wind)
#define VERTEX_COLOUR  @shPropertyBool(vertex_colour)
#define TWOSIDE_DIFFUSE  @shPropertyBool(twoside_diffuse)
#define ROAD_BLEND  @shPropertyBool(road_blend)
#define WATER_PARTICLES_LIT  @shPropertyBool(water_particles_lit)


#ifdef SH_VERTEX_SHADER

	//	  VERTEX
	//........................................................................................

	SH_BEGIN_PROGRAM
		shUniform(float4x4, wvp)  @shAutoConstant(wvp, worldviewproj_matrix)
		shVertexInput(float2, uv0)
		shOutput(float4, UV)
		shNormalInput(float4)
		
#if MRT
		shUniform(float4x4, wvMat)  @shAutoConstant(wvMat, worldview_matrix)
		shUniform(float, far)  @shAutoConstant(far, far_clip_distance)
		shOutput(float4, viewNormal)
#endif


#if SOFT_PARTICLES
		shOutput(float3, screenPosition)
#endif
 
#if VERTEX_COLOUR || ROAD_BLEND
		shColourInput(float4)
		shOutput(float4, vertexColour)
#endif

#if NORMAL_MAP
		shTangentInput(float3)
		shOutput(float3, tangentPassthrough)
#endif

#if TREE_WIND

		shUniform(float, windTimer)  @shSharedParameter(windTimer)
		shVertexInput(float4, uv1)  // windParams
		shVertexInput(float4, uv2)  // originPos
#endif

#if INSTANCING
		shVertexInput(float4, uv1)
		shVertexInput(float4, uv2)
		shVertexInput(float4, uv3)
		shUniform(float4x4, viewProjMatrix) @shAutoConstant(viewProjMatrix, viewproj_matrix)
#endif
 
#if GRASS_WIND

		shUniform(float, grassTimer)  @shSharedParameter(grassTimer)
		shUniform(float, grassFrequency)  @shSharedParameter(grassFrequency)
		shUniform(float4, grassDirection)  @shSharedParameter(grassDirection)

		shUniform(float4x4, world)  @shAutoConstant(world, world_matrix)
		shUniform(float4, posSph0)  @shSharedParameter(posSph0)  // car grass colision spheres pos,r^2
		shUniform(float4, posSph1)  @shSharedParameter(posSph1)
#endif

		shOutput(float3, normalPassthrough)
		shOutput(float4, objSpacePositionPassthrough)

#if SHADOWS
	 @shForeach(3)
		shOutput(float4, lightSpacePos@shIterator)
        #if INSTANCING
        shUniform(float4x4, texViewProjMatrix@shIterator) @shAutoConstant(texViewProjMatrix@shIterator, texture_viewproj_matrix, @shIterator)
        #else
		shUniform(float4x4, texWorldViewProjMatrix@shIterator)  @shAutoConstant(texWorldViewProjMatrix@shIterator, texture_worldviewproj_matrix,  @shIterator)
        #endif
	 @shEndForeach
#endif


	//..............................................................................................................................

	SH_START_PROGRAM
	{
	
		float4 position = shInputPosition;

#if INSTANCING
	    float4x4 worldMatrix;
	    worldMatrix[0] = uv1;
	    worldMatrix[1] = uv2;
	    worldMatrix[2] = uv3;
	    worldMatrix[3] = float4(0,0,0,1);
	    
	    #if SH_GLSL
	    worldMatrix = transpose(worldMatrix);
	    #endif


	    float4 worldPos   = shMatrixMult(worldMatrix, shInputPosition);
	    float3 worldNorm  = shMatrixMult(worldMatrix, float4(normal.xyz, 0)).xyz;
	    normalPassthrough = worldNorm.xyz;
#endif

#if TREE_WIND
		//float radiusCoeff = windParams.x;
		//float heightCoeff = windParams.y;
		//float factorX = windParams.z;
		//float factorY = windParams.w;

		position.y += sin(windTimer + uv2.z + position.y + position.x) * uv1.x * uv1.x * uv1.w;
		position.x += sin(windTimer + uv2.z ) * uv1.y * uv1.y * uv1.z;
#endif


		//  grass
#if GRASS_WIND
		float4 opos = shMatrixMult(world, position);
		float oldposx = position.x;
		if (uv0.y == 0.0f)
		{
			float offset = sin(grassTimer + oldposx * grassFrequency);
			position += grassDirection * offset;
			
   			#if 1
			///()  grass deform under car
			// sphere:  (x - x0)^2 + (y - y0)^2 + (z - z0)^2 = r^2
			float r = posSph0.w;  // sphere radius^2
			if (r > 0.f)
			{			
				float3 sph = posSph0.xyz;  // sphere pos
				float dx = opos.x - sph.x,  dz = opos.z - sph.z; //, dy = opos.y - sph.y;
				dx *= dx;  dz *= dz;  //dy *= dy;
				//if (dx + dy + dz < r)  // sphere
				if (dx + dz < r)  // circle (very high grasses too)
				{
					float y_on_sph = sph.y - sqrt(r - dx - dz);
					if (position.y > y_on_sph)
					{	position.y = y_on_sph;  opos.y = y_on_sph;  }
				}
				sph = posSph1.xyz;  // 2nd
				dx = opos.x - sph.x;  dz = opos.z - sph.z;
				dx *= dx;  dz *= dz;
				if (dx + dz < r)
				{
					float y_on_sph = sph.y - sqrt(r - dx - dz);
					if (position.y > y_on_sph)
						position.y = y_on_sph;
				}
			}
			#endif
		}
#endif

#if INSTANCING
        shOutputPosition = shMatrixMult(viewProjMatrix, worldPos);
#else
		shOutputPosition = shMatrixMult(wvp, position);
#endif


#if SOFT_PARTICLES
		screenPosition = float3(shOutputPosition.xy, shOutputPosition.w);
#endif
	
		UV.xy = uv0;

		normalPassthrough = normal.xyz;


#if VERTEX_COLOUR || ROAD_BLEND
		vertexColour = colour;
#endif

#ifdef NEED_DEPTH
		UV.z = shOutputPosition.z;
#endif

#if MRT
		UV.w = length(shMatrixMult(wvMat, position).xyz) / far;
		viewNormal = shMatrixMult(wvMat, float4(normal.xyz, 0));
#endif

		objSpacePositionPassthrough = position;

#if NORMAL_MAP
		tangentPassthrough = tangent;
#endif


#if SHADOWS
	 @shForeach(3)
        #if INSTANCING
        lightSpacePos@shIterator = shMatrixMult(texViewProjMatrix@shIterator, worldPos);
        #else
		lightSpacePos@shIterator = shMatrixMult(texWorldViewProjMatrix@shIterator, position);
        #endif
	 @shEndForeach
#endif
	}


#else

	//	  FRAGMENT
	//----------------------------------------------------------------------------------------

	SH_BEGIN_PROGRAM
		shSampler2D(diffuseMap)
		shInput(float4, UV)
		
#if MRT
		shInput(float4, viewNormal)
		shDeclareMrtOutput(1)
		shDeclareMrtOutput(2)
#endif

#if (MRT) || (SOFT_PARTICLES)
		shUniform(float, far)  @shAutoConstant(far, far_clip_distance)
#endif

#if SOFT_PARTICLES
		shInput(float3, screenPosition)
		shSampler2D(sceneDepth)
		shUniform(float4, viewportSize)  @shAutoConstant(viewportSize, viewport_size)
		shUniform(float, flip)  @shAutoConstant(flip, render_target_flipping)
#endif

#if (SOFT_PARTICLES) || (ENV_MAP)
		shUniform(float3, camPosWS)  @shAutoConstant(camPosWS, camera_position)
#endif


#if VERTEX_COLOUR || ROAD_BLEND
		shInput(float4, vertexColour)
#endif
		
#ifdef NEED_WORLD_MATRIX
		shUniform(float4x4, worldMatrix)  @shAutoConstant(worldMatrix, world_matrix)
#endif
		
		
#if CAR_PAINT_MAP
		shSampler2D(carPaintMap)
		shUniform(float3, carColour)

		shUniform(float, glossiness)
		shUniform(float, reflectiveness)

	    shUniform(float3, fresnelScaleBiasPower2)  @shUniformProperty3f(fresnelScaleBiasPower2, fresnelScaleBiasPower2)
		shUniform(float4, specular2)  @shUniformProperty4f(specular2, specular2)
#endif
	
		
#if ALPHA_MAP
		shSampler2D(alphaMap)
#endif

#if NORMAL_MAP
		shSampler2D(normalMap)
		shInput(float3, tangentPassthrough)
		shUniform(float, bumpScale)  @shUniformProperty1f(bumpScale, bump_scale)
#endif

#if ENV_MAP
		shSamplerCube(envMap)
		
	#if REFL_MAP
		shSampler2D(reflMap)
	#endif
		
	#if FRESNEL
		shUniform(float3, fresnelScaleBiasPower)  @shUniformProperty3f(fresnelScaleBiasPower, fresnelScaleBiasPower)
	#else
		shUniform(float, reflAmount)	 @shUniformProperty1f(reflAmount, refl_amount)
	#endif

	#if SPECULAR_ALPHA
		shUniform(float4, env_alpha)  @shUniformProperty4f(env_alpha, env_alpha)
	#endif
#endif

#if SPEC_MAP
		shSampler2D(specMap)
#endif

		shInput(float3, normalPassthrough)
		shInput(float4, objSpacePositionPassthrough)

		shUniform(float4, lightAmbient)		 @shAutoConstant(lightAmbient, ambient_light_colour)
		shUniform(float4, materialAmbient)	  @shAutoConstant(materialAmbient, surface_ambient_colour)
		shUniform(float4, materialDiffuse)	  @shAutoConstant(materialDiffuse, surface_diffuse_colour)
		shUniform(float4, materialSpecular)	 @shAutoConstant(materialSpecular, surface_specular_colour)
		shUniform(float4, materialEmissive)	 @shAutoConstant(materialEmissive, surface_emissive_colour)
		shUniform(float, materialShininess)	 @shAutoConstant(materialShininess, surface_shininess)
		
		shUniform(float4, lightPosObjSpace)	 @shAutoConstant(lightPosObjSpace, light_position_object_space)
		shUniform(float4, lightSpecular)		@shAutoConstant(lightSpecular, light_specular_colour)
		shUniform(float4, lightDiffuse)		 @shAutoConstant(lightDiffuse, light_diffuse_colour)

		shUniform(float3, camPosObjSpace)	   @shAutoConstant(camPosObjSpace, camera_position_object_space)
		
#if FOG
        shUniform(float4, fogParams)  @shAutoConstant(fogParams, fog_params)
		shUniform(float4, fogColorSun)   @shSharedParameter(fogColorSun)
		shUniform(float4, fogColorAway)  @shSharedParameter(fogColorAway)
		shUniform(float4, fogColorH)     @shSharedParameter(fogColorH)
		shUniform(float4, fogParamsH)    @shSharedParameter(fogParamsH)

		shUniform(float4, fogFluidH)     @shSharedParameter(fogFluidH)
		shUniform(float4, fogFluidClr)   @shSharedParameter(fogFluidClr)
#endif


#if TERRAIN_LIGHT_MAP
		shUniform(float, terrainWorldSize)   @shSharedParameter(terrainWorldSize)
		shSampler2D(terrainLightMap)
	#if TERRAIN_LIGHT_MAP_TOGGLEABLE
		shUniform(float, enableTerrainLightMap)
	#endif
#endif

#if GRASS_WIND
		shUniform(float, grassFadeRange)  @shSharedParameter(grassFadeRange)
#endif


#if SELECTED_GLOW
		shUniform(float, isSelected)  @shAutoConstant(isSelected, custom, 1)
		shUniform(float, time)  @shAutoConstant(time, time, 1)
#endif

#if SHADOWS
	 @shForeach(3)
		shInput(float4, lightSpacePos@shIterator)
		shSampler2D(shadowMap@shIterator)
		shUniform(float2, invShadowmapSize@shIterator)   @shAutoConstant(invShadowmapSize@shIterator, inverse_texture_size, @shIterator)
	 @shEndForeach
		shUniform(float3, pssmSplitPoints)   @shSharedParameter(pssmSplitPoints)
#endif

#if SHADOWS
		shUniform(float4, shadowFar_fadeStart)  @shSharedParameter(shadowFar_fadeStart)
		shUniform(float, shadowBias)   @shUniformProperty1f(shadowBias, shadowBias)
#endif


	//------------------------------------------------------------------------------------------------------------------------------

	SH_START_PROGRAM
	{
		shOutputColour(0) = shSample(diffuseMap, UV.xy);
		
#if CAR_PAINT_MAP
		shOutputColour(0).xyz = shLerp ( shOutputColour(0).xyz, shSample(carPaintMap, UV.xy).r * carColour, 1 - shOutputColour(0).a);
#endif

#if VERTEX_COLOUR
		shOutputColour(0) *= vertexColour;
#endif

#ifdef NEED_DEPTH
		float depth = UV.z;
#endif

		//  normal
		float3 normal = normalize(normalPassthrough);

#if NORMAL_MAP

		float3 binormal = cross(tangentPassthrough.xyz, normal.xyz);
		float3x3 tbn = float3x3(tangentPassthrough.xyz * bumpScale, binormal * bumpScale, normal.xyz);
		
		#if SH_GLSL
			tbn = transpose(tbn);
		#endif

		float3 TSnormal = shSample(normalMap, UV.xy).xyz * 2 - 1;
		
		normal = normalize (shMatrixMult( transpose(tbn), TSnormal ));
#endif

#if WATER_PARTICLES_LIT
		// *** water particles only  average, don't color
		float ambAvg = lightAmbient.x+lightAmbient.y+lightAmbient.z;
		float3 ambient = materialAmbient.xyz * ambAvg / 3.f;
#else   
		float3 ambient = materialAmbient.xyz * lightAmbient.xyz;
#endif
		
	
		//  shadows
#if SHADOWS
		float shadow = pssmDepthShadow (lightSpacePos0, invShadowmapSize0, shadowMap0, lightSpacePos1, invShadowmapSize1, shadowMap1, lightSpacePos2, invShadowmapSize2, shadowMap2, depth, pssmSplitPoints, shadowBias);
#endif

#if SHADOWS
		float fadeRange = shadowFar_fadeStart.x - shadowFar_fadeStart.y;
		float fade = 1-((depth - shadowFar_fadeStart.y) / fadeRange);
		shadow = (depth > shadowFar_fadeStart.x) ? 1.0 : ((depth > shadowFar_fadeStart.y) ? 1.0-((1.0-shadow)*fade) : shadow);
#endif

#if !(SHADOWS)
		float shadow = 1.0;
#endif


#if TERRAIN_LIGHT_MAP
		float shadowingLM;
		float2 worldPos = shMatrixMult(worldMatrix, float4(objSpacePositionPassthrough.xyz, 1)).xz;
		float2 lmTexCoord = (worldPos / terrainWorldSize) + 0.5;
		shadowingLM = shSample(terrainLightMap, lmTexCoord).x;
		#if TERRAIN_LIGHT_MAP_TOGGLEABLE
			shadow = min(shadow, (enableTerrainLightMap == 1) ? shadowingLM : 1.0);
		#else
			shadow = min(shadow, shadowingLM);
		#endif
#endif


		//  diffuse, specular
		float3 lightDir = normalize(lightPosObjSpace.xyz);

#if GRASS_WIND
		float NdotL = 1;
#else
		#if TWOSIDE_DIFFUSE
			float NdotL = abs(dot(normal, lightDir));
		#else
			float NdotL = max(dot(normal, lightDir), 0);
		#endif
#endif
		float3 diffuse = materialDiffuse.xyz * lightDiffuse.xyz * NdotL * shadow;
	
		float3 eyeDir = normalize(camPosObjSpace.xyz - objSpacePositionPassthrough.xyz);
		float3 halfAngle = normalize (lightDir + eyeDir);
		
		#if ENV_MAP		
			float spec_mul = 4;  //x4
		#else
			float spec_mul = 1;
		#endif
		
		shOutputColour(0).xyz *= (ambient + diffuse + materialEmissive.xyz);
		
		
		//  specular
		float3 specular = float3(0,0,0);
		if (NdotL > 0 && shadow > 0)
		{
		#if TWOSIDE_DIFFUSE
			float specDot = abs(dot(normal, halfAngle));
		#else
			float specDot = max(dot(normal, halfAngle), 0);
		#endif

		#if CAR_PAINT_MAP
			float smul = shLerp(1, specular2.w / materialShininess, glossiness);
			float shininess = shLerp(materialShininess, specular2.w, glossiness);
			float3 matSpec = shLerp(materialSpecular.xyz, specular2.xyz, glossiness);
		#else
			float shininess = materialShininess;
			float3 matSpec = materialSpecular.xyz;
			float smul = 1;
		#endif

		#if !SPEC_MAP
			specular = pow(specDot, spec_mul * shininess) * matSpec;
			
			#if CAR_PAINT_MAP
			specular += pow(specDot, 1024) * matSpec;  // sun on car body
			#endif
		#else
			float4 specTex = shSample(specMap, UV.xy);
			#if !SPECMAP_RGB						/* spec_mul */ 
			specular = pow(specDot, specTex.a * 255 * smul) * specTex.xyz * matSpec;
			#else
			specular = pow(specDot, shininess * smul) * specTex.xyz * matSpec;
			#endif
			
			#if CAR_PAINT_MAP
			specular += pow(specDot, 1024) * specTex.xyz * matSpec;
			#endif
		#endif
		shOutputColour(0).xyz += specular * lightSpecular.xyz * shadow;
		}
		
		
		//  reflection
#if ENV_MAP		   
		float3 r = reflect( -eyeDir, normal );
		r = normalize(shMatrixMult(worldMatrix, float4(r, 0)).xyz); 
		
		r.z = -r.z;
		#if SPECULAR_ALPHA
		float4 envColor = shCubicSample(envMap, r) * env_alpha.x;
		#else
		float4 envColor = shCubicSample(envMap, r);
		#endif
		
		float reflectionFactor = 1;
		
		#if REFL_MAP
			reflectionFactor *= shSample(reflMap, UV.xy).r;
		#endif
		
		#if FRESNEL
			float facing = 1.0 - max(abs(dot(-eyeDir, normal)), 0);
			#if CAR_PAINT_MAP
				float3 fSBP = shLerp(fresnelScaleBiasPower, fresnelScaleBiasPower2, glossiness);
				reflectionFactor *= reflectiveness;
			#else
				float3 fSBP = fresnelScaleBiasPower;
			#endif
			reflectionFactor *= shSaturate(fSBP.y + fSBP.x * pow(facing, fSBP.z));
		#else
			float facing = reflAmount;
			reflectionFactor *= reflAmount;
		#endif
		
		shOutputColour(0).xyz = shLerp(shOutputColour(0).xyz, envColor.xyz, reflectionFactor);
#endif


#if FOG
        float worldPosY = shMatrixMult(worldMatrix, float4(objSpacePositionPassthrough.xyz, 1)).y;

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
		// old FOG
		//float fogValue = shSaturate((depth - fogParams.y) * fogParams.w);
		//shOutputColour(0).xyz = shLerp (shOutputColour(0).xyz, fogColour, fogValue);


		//  alpha
		shOutputColour(0).a *= materialDiffuse.a;
		
#if ALPHA_MAP
		shOutputColour(0).a *= shSample(alphaMap, float2(UV.x, UV.y * 0.025)).r;
#endif

#if SPECULAR_ALPHA
		//  bump alpha with specular
		#if ENV_MAP																	// par                // par
		shOutputColour(0).a = min(shOutputColour(0).a + specular.g + pow(envColor.g * env_alpha.y, env_alpha.z) * facing * env_alpha.w, 1);
		#else
		shOutputColour(0).a = min(shOutputColour(0).a + specular.g, 1);
		#endif
#endif


#if GRASS_WIND
		//  grass distance fading
		float dist = distance(camPosObjSpace.xz, objSpacePositionPassthrough.xz);
		shOutputColour(0).a *= (2.0f - (2.0f * dist / grassFadeRange));
#endif

#if SOFT_PARTICLES
		float2 screenUV = screenPosition.xy / screenPosition.z;
		screenUV = screenUV * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
		screenUV += (viewportSize.zw) * 0.5;
		screenUV.y =(1-shSaturate(flip))+flip*screenUV.y;
		float depthTex = shSample(sceneDepth, screenUV).x * far;
		float4 worldPos = shMatrixMult(worldMatrix, float4(objSpacePositionPassthrough.xyz, 1));
		float distanceToPixel = length(worldPos.xyz - camPosWS.xyz);
		float thickness = 0.5;
		float tNear = distanceToPixel - thickness;
		float tFar = distanceToPixel + thickness;
		float depthAlpha = shSaturate(depthTex - distanceToPixel);
		shOutputColour(0).a *= depthAlpha;
#endif


#if ROAD_BLEND 
		shOutputColour(0).a *= vertexColour.b;
#endif

#if SELECTED_GLOW
		shOutputColour(0).xyzw += isSelected * (float4(0.0, 0.4, 1.0, 0.5) * (0.5 + 0.1 * cos(4.0 * time)));
#endif
		//shOutputColour(0).xyz = shOutputColour(0).xyz * 0.001 + normal.xyz;  // normal test

#if MRT
		shOutputColour(1) = float4(UV.w, normalize(viewNormal.xyz));
		shOutputColour(2) = float4(UV.z / far, 0, UV.z / objSpacePositionPassthrough.w, MOTIONBLUR_MASK);
#endif

	}

#endif
