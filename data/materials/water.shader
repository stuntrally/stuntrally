#include "core.h"

// Inspired by Blender GLSL Water by martinsh ( http://devlog-martinsh.blogspot.de/2012/07/waterundewater-shader-wip.html )


#define SHADOWS @shGlobalSettingBool(shadows_pssm) && @shPropertyBool(receives_shadows)
#define SHADOWS_DEPTH @shGlobalSettingBool(shadows_depth)

#if SHADOWS
    #include "shadows.h"
#endif

#define EDITOR @shGlobalSettingBool(editor)

#if EDITOR
	#define WATERDEPTH_SWITCH
#endif

#ifdef SH_VERTEX_SHADER

    // ------------------------------------- VERTEX ---------------------------------------

	SH_BEGIN_PROGRAM
		shUniform(float4x4, wvp)				@shAutoConstant(wvp, worldviewproj_matrix)
		shVertexInput(float2, uv0)
		shOutput(float2, UV)
		
		shOutput(float3, screenCoordsPassthrough)
		shOutput(float4, position)
		shOutput(float, depth)

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
	   
	   
		#if !SH_GLSL
		float4x4 scalemat = float4x4(0.5, 0,    0,   0.5,
									 0,   -0.5, 0,   0.5,
									 0,   0,    0.5, 0.5,
									 0,   0,    0,   1   );
		#else							
		mat4 scalemat = mat4(0.5, 0.0, 0.0, 0.0, 
							 0.0,-0.5, 0.0, 0.0,
							 0.0, 0.0, 0.5, 0.0,
							 0.5, 0.5, 0.5, 1.0);
		#endif
										
		float4 texcoordProj = shMatrixMult(scalemat, shOutputPosition);
		screenCoordsPassthrough = float3(texcoordProj.x, texcoordProj.y, texcoordProj.w);
		
		position = shInputPosition;
		
		depth = shOutputPosition.z;

#if SHADOWS
    @shForeach(3)
        lightSpacePos@shIterator = shMatrixMult(texWorldViewProjMatrix@shIterator, position);
    @shEndForeach
#endif
	}

#else
    // ----------------------------------- FRAGMENT ------------------------------------------

	// tweakables

		#define VISIBILITY 1500.0				   // how far you can look through water

		#define ABBERATION 0.001					// chromatic abberation amount

		//#define SCATTER_AMOUNT 3.0				  // amount of sunlight scattering
		//#define SCATTER_COLOUR float3(0.0,1.0,0.95) // colour of sunlight scattering
		
		#define SUN_EXT float3(0.45, 0.55, 0.68)	//sunlight extinction
		
	// ---------------------------------------------------------------
	
	
	#define SCREEN_REFLECTION @shGlobalSettingBool(water_reflect)
	#define SCREEN_REFRACTION @shGlobalSettingBool(water_refract)


	float fresnel_dielectric(float3 Incoming, float3 Normal, float eta)
	{
		/* compute fresnel reflectance without explicitly computing
		   the refracted direction */
		float c = abs(dot(Incoming, Normal));
		float g = eta * eta - 1.0 + c * c;
		float result;

		if(g > 0.0) {
			g = sqrt(g);
			float A =(g - c)/(g + c);
			float B =(c *(g + c)- 1.0)/(c *(g - c)+ 1.0);
			result = 0.5 * A * A *(1.0 + B * B);
		}
		else
			result = 1.0;  /* TIR (no refracted component) */

		return result;
	}


	SH_BEGIN_PROGRAM
		shInput(float2, UV)
		shInput(float3, screenCoordsPassthrough)
		shInput(float4, position)
		shInput(float, depth)
		
		
		shUniform(float2, choppyness_scale)  @shUniformProperty2f(choppyness_scale, choppyness_scale)
		#define WAVE_CHOPPYNESS  choppyness_scale.x
		#define WAVE_SCALE  choppyness_scale.y
		
		shUniform(float4, smallWaves_midWaves)  @shUniformProperty4f(smallWaves_midWaves, smallWaves_midWaves)
		#define SMALL_WAVES_X  smallWaves_midWaves.x
		#define SMALL_WAVES_Y  smallWaves_midWaves.y
		#define MID_WAVES_X  smallWaves_midWaves.z
		#define MID_WAVES_Y  smallWaves_midWaves.w
		
		shUniform(float2, bigWaves)  @shUniformProperty2f(bigWaves, bigWaves)
		#define BIG_WAVES_X  bigWaves.x
		#define BIG_WAVES_Y  bigWaves.y
		
		shUniform(float3, bump)  @shUniformProperty3f(bump, bump)
		#define BUMP  bump.x
		#define REFL_BUMP  bump.y
		#define REFR_BUMP  bump.z
		shUniform(float3, bump2SpecPowerMul)  @shUniformProperty3f(bump2SpecPowerMul, bump2SpecPowerMul)
		
		shUniform(float4, specColourAndPower)  @shUniformProperty4f(specColourAndPower, specColourAndPower)
		shUniform(float4, waterClr)  @shUniformProperty4f(waterClr, colour)
		shUniform(float, fresnelMultiplier)  @shUniformProperty1f(fresnelMultiplier, fresnelMultiplier)
		shUniform(float, far)  @shAutoConstant(far, far_clip_distance)

		shUniform(float4, reflectColour)  @shUniformProperty4f(reflectColour, reflectColour)
		shUniform(float4, refractColour)  @shUniformProperty4f(refractColour, refractColour)
	
		#if SCREEN_REFLECTION
			shSampler2D(reflectionMap)
		#else
			shSampler2D(reflectionSkyMap)
		#endif

		#if SCREEN_REFRACTION
			shSampler2D(refractionMap)
		#endif
		shSampler2D(normalMap)
		shSampler2D(depthMap)
		
		shUniform(float3, windDir_windSpeed)  @shSharedParameter(windDir_windSpeed)
		shUniform(float, speed)  @shUniformProperty1f(speed, speed)
		#define WIND_SPEED  windDir_windSpeed.z
		#define WIND_DIR  windDir_windSpeed.xy
		
		shUniform(float, waterTimer)  @shSharedParameter(waterTimer)
		shUniform(float2, waterSunFade_sunHeight)  @shSharedParameter(waterSunFade_sunHeight)
		
		shUniform(float4x4, worldMatrix)  @shAutoConstant(worldMatrix, world_matrix)
		shUniform(float, terrainWorldSize)  @shSharedParameter(terrainWorldSize)
		
		shUniform(float, renderTargetFlipping)  @shAutoConstant(renderTargetFlipping, render_target_flipping)
		
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
		shUniform(float4, sunPosition)  @shAutoConstant(sunPosition, light_position, 0)
		shUniform(float4, lightPosObjSpace)	 @shAutoConstant(lightPosObjSpace, light_position_object_space)

        shUniform(float4, lightAmbient)  @shAutoConstant(lightAmbient, ambient_light_colour)
        shUniform(float4, lightDiffuse)  @shAutoConstant(lightDiffuse, light_diffuse_colour)
        shUniform(float4, lightSpecular)  @shAutoConstant(lightSpecular, light_specular_colour)
		
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

#ifdef WATERDEPTH_SWITCH
	shUniform(float, waterDepth) @shSharedParameter(waterDepth)
#endif

	//------------------------------------------------------------------------------------------------------------------------------

	SH_START_PROGRAM
	{

		// water depth
		float4 worldPos = shMatrixMult(worldMatrix, position);
		float2 depthUV = float2(-worldPos.z / terrainWorldSize + 0.5f, worldPos.x / terrainWorldSize + 0.5f);
		float4 depthTex = shSample(depthMap, depthUV);

#ifdef WATERDEPTH_SWITCH
		if (waterDepth == 0)
			depthTex = float4(1.0, 1.0, 0.0, 0.0);
#endif
		
		// no need to render below terrain
		if (depthTex.x == 0)
			discard;

		float2 screenCoords = screenCoordsPassthrough.xy / screenCoordsPassthrough.z;
		screenCoords.y = (1-shSaturate(renderTargetFlipping))+renderTargetFlipping*screenCoords.y;


		float2 nCoord = float2(0,0);
	  	float timer = waterTimer * speed;
	  	
nCoord = UV * (WAVE_SCALE * 0.05)+ WIND_DIR * timer * (WIND_SPEED*0.04);											float3 normal0 = 2.0 * shSample(normalMap, nCoord + float2(-timer*0.015,-timer*0.005)).rgb - 1.0;
nCoord = UV * (WAVE_SCALE * 0.1) + WIND_DIR * timer * (WIND_SPEED*0.08)-(normal0.xy/normal0.zz)*WAVE_CHOPPYNESS;	float3 normal1 = 2.0 * shSample(normalMap, nCoord + float2(+timer*0.020,+timer*0.015)).rgb - 1.0;
nCoord = UV * (WAVE_SCALE * 0.25)+ WIND_DIR * timer * (WIND_SPEED*0.07)-(normal1.xy/normal1.zz)*WAVE_CHOPPYNESS;	float3 normal2 = 2.0 * shSample(normalMap, nCoord + float2(-timer*0.04,-timer*0.03)).rgb - 1.0;
nCoord = UV * (WAVE_SCALE * 0.5) + WIND_DIR * timer * (WIND_SPEED*0.09)-(normal2.xy/normal2.zz)*WAVE_CHOPPYNESS;	float3 normal3 = 2.0 * shSample(normalMap, nCoord + float2(+timer*0.03,+timer*0.04)).rgb - 1.0;
nCoord = UV * (WAVE_SCALE * 1.0) + WIND_DIR * timer * (WIND_SPEED*0.4)-(normal3.xy/normal3.zz)*WAVE_CHOPPYNESS;		float3 normal4 = 2.0 * shSample(normalMap, nCoord + float2(-timer*0.02,+timer*0.1)).rgb - 1.0;
nCoord = UV * (WAVE_SCALE * 2.0) + WIND_DIR * timer * (WIND_SPEED*0.7)-(normal4.xy/normal4.zz)*WAVE_CHOPPYNESS;		float3 normal5 = 2.0 * shSample(normalMap, nCoord + float2(+timer*0.1,-timer*0.06)).rgb - 1.0;
		

		float3 normal = (normal0 * BIG_WAVES_X   + normal1 * BIG_WAVES_Y +
						 normal2 * MID_WAVES_X   + normal3 * MID_WAVES_Y +
						 normal4 * SMALL_WAVES_X + normal5 * SMALL_WAVES_Y).xzy;
		
		// normal for sunlight scattering					
		float3 lNormal = (normal0 * BIG_WAVES_X*0.5   + normal1 * BIG_WAVES_Y*0.5 +
						  normal2 * MID_WAVES_X*0.2   + normal3 * MID_WAVES_Y*0.2 +
						  normal4 * SMALL_WAVES_X*0.1 + normal5 * SMALL_WAVES_Y*0.1).xzy;

		float3 normalX2 = normalize(float3(normal.x * bump2SpecPowerMul.x, normal.y, normal.z * bump2SpecPowerMul.x));  // n2
		normal  = normalize(float3(normal.x * BUMP, normal.y, -normal.z * BUMP));
		lNormal = normalize(float3(lNormal.x * BUMP, lNormal.y, -lNormal.z * BUMP));
		
		
		float3 lVec = normalize(sunPosition.xyz);
		float3 vVec = normalize(position.xyz - cameraPos.xyz);
		
		
		float isUnderwater = (cameraPos.y > 0) ? 0.0 : 1.0;

	   
		// sunlight scattering
		/*
		float3 pNormal = float3(0,1,0);
		float3 lR = reflect(lVec, lNormal);
		float3 llR = reflect(lVec, pNormal);
		
		float s = shSaturate(dot(lR, vVec)*2.0-1.2);

		float lightScatter = shSaturate(dot(-lVec,lNormal)*0.7+0.3) * s * SCATTER_AMOUNT * waterSunFade_sunHeight.x * shSaturate(1.0-exp(-waterSunFade_sunHeight.y));
		float3 scatterColour = shLerp(float3(SCATTER_COLOUR)*float3(1.0,0.4,0.0), SCATTER_COLOUR, shSaturate(1.0-exp(-waterSunFade_sunHeight.y*SUN_EXT)));
		*/


        //  Shadows
		#if SHADOWS
            float shadow = pssmDepthShadow (lightSpacePos0, invShadowmapSize0, shadowMap0, lightSpacePos1, invShadowmapSize1, shadowMap1, lightSpacePos2, invShadowmapSize2, shadowMap2, depth, pssmSplitPoints, 0.f);
		#endif

		#if SHADOWS
            float fadeRange = shadowFar_fadeStart.x - shadowFar_fadeStart.y;
            float fade = 1-((depth - shadowFar_fadeStart.y) / fadeRange);
            shadow = (depth > shadowFar_fadeStart.x) ? 1.0 : ((depth > shadowFar_fadeStart.y) ? 1.0-((1.0-shadow)*fade) : shadow);
		#endif

		#if !(SHADOWS)
            float shadow = 1.0;
		#endif
		

	//#if TERRAIN_LIGHT_MAP
	//	float shadowingLM;
	//	float2 worldPos = shMatrixMult(wMat, float4(objSpacePositionPassthrough.xyz, 1)).xz;
	//	float2 lmTexCoord = (worldPos / terrainWorldSize) + 0.5;
	//	shadowingLM = shSample(terrainLightMap, lmTexCoord).x;
 //       #if TERRAIN_LIGHT_MAP_TOGGLEABLE
	//	shadow = min(shadow, (enableTerrainLightMap == 1) ? shadowingLM : 1.0);
	//	#else
	//	shadow = min(shadow, shadowingLM);
	//	#endif
	//#endif


		//  fresnel
		float ior = (cameraPos.y>0)?(1.333/1.0):(1.0/1.333); //air to water; water to air
		float fresnel = fresnel_dielectric(-vVec, normal, ior);
		
		fresnel = shSaturate(shSaturate(fresnel) * fresnelMultiplier);

	
		//  reflection
		float3 R = reflect(vVec, normal);
		#if SCREEN_REFLECTION
			float3 reflection = reflectColour.rgb * shSample(reflectionMap, screenCoords+(normal.xz*REFL_BUMP)).rgb;
		#else
			float3 reflNormal = normalize(float3(normal.x * REFL_BUMP*6, normal.y, normal.z * REFL_BUMP*6));
			//float3 reflNormal = normalize(float3(0, normal.y, 0));  //test
			R = reflect(vVec, reflNormal);
			const float PI = 3.1415926536f;
			float2 refl2;
			#if SH_GLSL
				refl2.x = /*(refl.x == 0) ? 0 :*/ ( (R.z < 0.0) ? atan(-R.z, R.x) : (2*PI - atan(R.z, R.x)) );
			#else
				refl2.x = /*(refl.x == 0) ? 0 :*/ ( (R.z < 0.0) ? atan2(-R.z, R.x) : (2*PI - atan2(R.z, R.x)) );
			#endif
			refl2.x = 1 - refl2.x / (2*PI);  // yaw   0..1
			refl2.y = 1 - asin(R.y) / PI*2;  // pitch 0..1
			float3 reflection = reflectColour.rgb * shSample(reflectionSkyMap, refl2).rgb;
		#endif
		
		
		float shoreFade = depthTex.r;  // smooth transition to shore
		
		//  Refraction
		#if SCREEN_REFRACTION
			float3 refraction = float3(0,0,0);
			refraction.r = shSample(refractionMap, (screenCoords-shoreFade*(normal.xz*REFR_BUMP))*1.0).r;
			refraction.g = shSample(refractionMap, (screenCoords-shoreFade*(normal.xz*REFR_BUMP))*1.0-(R.xy*ABBERATION)).g;
			refraction.b = shSample(refractionMap, (screenCoords-shoreFade*(normal.xz*REFR_BUMP))*1.0-(R.xy*ABBERATION*2.0)).b;

			 // brighten up the refraction underwater  why?
			//refraction = (cameraPos.y < 0) ? shSaturate(refraction * 1.5) : refraction;
		#endif
	
	
		//  specular
		float specular = pow(max(dot(R, lVec), 0.0), specColourAndPower.w) * shadow;
		float3 waterColour = waterClr.rgb * shSaturate(lightDiffuse.rgb);
		#if !SCREEN_REFRACTION
			float3 refraction = shLerp( refractColour.rgb, waterColour.rgb, 0.5);  //old
		#endif

		//  specular2
		R = reflect(vVec, normalX2);  // n2
		specular += pow(max(dot(R, lVec), 0.0), bump2SpecPowerMul.y) * bump2SpecPowerMul.z * (0.3 + 0.7 * shadow);
		
		reflection = shLerp( waterColour, reflection,  reflectColour.a);  // reflection instensity

		
		//  FINAL Lerp							//_ no refraction at distant
		//---------------------------------------------------------------
		float depthAmount = depthTex.g * shLerp( refractColour.a, 1, fresnel) * (1-isUnderwater);  // deep water

		float3 clr = refractColour.rgb * shLerp( refraction, waterColour,  depthAmount);
		refraction = waterClr.a > 0.5 ? shLerp( clr,  waterColour, refractColour.a) : clr;  // no refraction inside mud

		float3 finCLR = shLerp( refraction, reflection,  fresnel);
		finCLR = shLerp( refraction, finCLR,  shoreFade);
		shOutputColour(0).rgb = finCLR * (0.6 + 0.4 * shadow);

		#if SCREEN_REFRACTION
			shOutputColour(0).a = shoreFade;
		#else
			//old  alpha if no refraction present (not in mud)
			shOutputColour(0).a = shLerp( shoreFade * (0.7 + 0.3 * depthAmount), shoreFade,  waterClr.a);
		#endif
		
		shOutputColour(0).rgb += specular * specColourAndPower.rgb * lightSpecular.rgb;
		//shOutputColour(0).rgb = shOutputColour(0).rgb*0.001 + waterClr.a * float3(1,1,1);//test
				

		//  fog
		if (isUnderwater == 1)  // todo if ?..
		{
			float waterSunGradient = dot(-vVec, -lVec);
			waterSunGradient = shSaturate(pow(waterSunGradient*0.7+0.3,2.0));  
			float3 waterSunColour = float3(0.0,1.0,0.85)*waterSunGradient * 0.5;
		   
			float waterGradient = dot(-vVec, float3(0.0,-1.0,0.0));
			waterGradient = clamp((waterGradient*0.5+0.5),0.2,1.0);
			float3 watercolour = (float3(0.0078, 0.5176, 0.700)+waterSunColour)*waterGradient*2.0;
			float3 waterext = float3(0.6, 0.9, 1.0);  //water extinction
			watercolour = shLerp( watercolour*0.3*waterSunFade_sunHeight.x, watercolour, shSaturate(1.0-exp(-waterSunFade_sunHeight.y*SUN_EXT)));
		
			float darkness = VISIBILITY*2.0;
			darkness = clamp((cameraPos.y+darkness)/darkness,0.2,1.0);
	
		
			float fog = shSaturate(length(cameraPos.xyz - position.xyz) / VISIBILITY);
			shOutputColour(0).xyz = shLerp( shOutputColour(0).xyz, watercolour * darkness, shSaturate(fog / waterext));
		}
		//else
		//{
		//#if FOG

		float3 lightDir = lightPosObjSpace.xyz; // directional
		float3 eyeDir = cameraPos.xyz - position.xyz;
		float worldPosY = worldPos.y;

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
		shOutputColour(0).a = shLerp( shOutputColour(0).a, 1,  flL);
		///_

		//#endif
		//}

	}


#endif
