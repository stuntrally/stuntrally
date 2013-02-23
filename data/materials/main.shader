#include "core.h"

#define FOG @shGlobalSettingBool(fog)

#define SHADOWS @shGlobalSettingBool(shadows_pssm) && @shPropertyBool(receives_shadows)
#define SHADOWS_DEPTH @shGlobalSettingBool(shadows_depth)

#define MRT @shPropertyBool(mrt_output) && @shGlobalSettingBool(mrt_output)

#if SHADOWS
    #include "shadows.h"
#endif

#if FOG || (SHADOWS) || MRT
#define NEED_DEPTH
#endif

#define ALPHA_MAP @shPropertyBool(alpha_map)
#define NORMAL_MAP @shPropertyBool(normal_map)
#define ENV_MAP @shPropertyBool(env_map)
#define FRESNEL @shPropertyBool(fresnel)
#define REFL_MAP @shPropertyBool(refl_map)
#define SPEC_MAP @shPropertyBool(spec_map)
#define CAR_PAINT_MAP @shPropertyBool(car_paint_map)
#define TERRAIN_LIGHT_MAP @shPropertyBool(terrain_light_map)
#define TERRAIN_LIGHT_MAP_TOGGLEABLE @shPropertyBool(terrain_light_map_toggleable)

#define BLINNPHONG @shPropertyEqual(lightmodel, blinnphong)
#define COOKTORRANCE @shPropertyEqual(lightmodel, cooktorrance)
#define WARD @shPropertyEqual(lightmodel, ward)
#define ASHIKHMANSHIRLEY @shPropertyEqual(lightmodel, ashikhmanshirley)

#define SOFT_PARTICLES (@shPropertyBool(soft_particles) && @shGlobalSettingBool(soft_particles))

#define SELECTED_GLOW @shGlobalSettingBool(editor)


#if (TERRAIN_LIGHT_MAP) || (ENV_MAP) || (SOFT_PARTICLES)
#define NEED_WORLD_MATRIX
#endif

#define SPECULAR_ALPHA @shPropertyBool(specular_alpha)

#define TREE_WIND @shPropertyBool(tree_wind)
#define GRASS_WIND @shPropertyBool(grass_wind)
#define VERTEX_COLOUR @shPropertyBool(vertex_colour)
#define TWOSIDE_DIFFUSE @shPropertyBool(twoside_diffuse)

#ifdef SH_VERTEX_SHADER

    // ------------------------------------- VERTEX ---------------------------------------

    SH_BEGIN_PROGRAM
        shUniform(float4x4, wvp) @shAutoConstant(wvp, worldviewproj_matrix)
        shVertexInput(float2, uv0)
        shOutput(float4, UV)
        shNormalInput(float4)
        
#if MRT
        shUniform(float4x4, wvMat) @shAutoConstant(wvMat, worldview_matrix)
        shUniform(float, far) @shAutoConstant(far, far_clip_distance)
        shOutput(float4, viewNormal)
#endif


#if SOFT_PARTICLES
		shOutput(float3, screenPosition)
#endif
 
#if VERTEX_COLOUR
        shColourInput(float4)
        shOutput(float4, vertexColour)
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

#if GRASS_WIND

        shUniform(float, grassTimer) @shSharedParameter(grassTimer)
        shUniform(float, grassFrequency) @shSharedParameter(grassFrequency)
        shUniform(float4, grassDirection) @shSharedParameter(grassDirection)
#endif

        shOutput(float3, normalPassthrough)
        shOutput(float4, objSpacePositionPassthrough)

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

#if GRASS_WIND
        float oldposx = position.x;
        if (uv0.y == 0.0f)
        {
            float offset = sin(grassTimer + oldposx * grassFrequency);
            position += grassDirection * offset;
        }
#endif

	    shOutputPosition = shMatrixMult(wvp, position);

#if SOFT_PARTICLES
		screenPosition = float3(shOutputPosition.xy, shOutputPosition.w);
#endif
    
	    UV.xy = uv0;

        normalPassthrough = normal.xyz;

#if VERTEX_COLOUR
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
        lightSpacePos@shIterator = shMatrixMult(texWorldViewProjMatrix@shIterator, position);
    @shEndForeach
#endif
    }

#else

    // ----------------------------------- FRAGMENT ------------------------------------------


    SH_BEGIN_PROGRAM
		shSampler2D(diffuseMap)
		shInput(float4, UV)
		
#if MRT
        shInput(float4, viewNormal)
        shDeclareMrtOutput(1)
        shDeclareMrtOutput(2)
#endif

#if (MRT) || (SOFT_PARTICLES)
        shUniform(float, far) @shAutoConstant(far, far_clip_distance)
#endif

#if SOFT_PARTICLES
        shInput(float3, screenPosition)
        shSampler2D(sceneDepth)
        shUniform(float4, viewportSize) @shAutoConstant(viewportSize, viewport_size)
        shUniform(float, flip) @shAutoConstant(flip, render_target_flipping)
#endif

#if (SOFT_PARTICLES) || (ENV_MAP)
        shUniform(float3, camPosWS) @shAutoConstant(camPosWS, camera_position)
#endif


#if VERTEX_COLOUR
        shInput(float4, vertexColour)
#endif
		
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
        shInput(float3, tangentPassthrough)
        shUniform(float, bumpScale) @shUniformProperty1f(bumpScale, bump_scale)
#endif
				
#if COOKTORRANCE
	shUniform(float, beckmann_term)    @shUniformProperty1f(beckmann_term, beckmann_term)
#endif
	
#if COOKTORRANCE || ASHIKHMANSHIRLEY
	shUniform(float, fresnel_term)    @shUniformProperty1f(fresnel_term, fresnel_term)
#endif
	
#if WARD || ASHIKHMANSHIRLEY
	shUniform(float, aniso_term_x)    @shUniformProperty1f(aniso_term_x, aniso_term_x)
	shUniform(float, aniso_term_y)    @shUniformProperty1f(aniso_term_y, aniso_term_y)
#endif
	
	shUniform(float, diffuse_mul)    @shUniformProperty1f(diffuse_mul, diffuse_mul)
	shUniform(float, spec_mul)    @shUniformProperty1f(spec_mul, spec_mul)

#if ENV_MAP
        shSamplerCube(envMap)
        
        #if REFL_MAP
        shSampler2D(reflMap)
        #endif
        
        #if FRESNEL
        shUniform(float3, fresnelScaleBiasPower) @shUniformProperty3f(fresnelScaleBiasPower, fresnelScaleBiasPower)
        #else
        shUniform(float, reflAmount)    @shUniformProperty1f(reflAmount, refl_amount)
        #endif
#endif

#if SPEC_MAP
        shSampler2D(specMap)
#endif

        shInput(float3, normalPassthrough)
        shInput(float4, objSpacePositionPassthrough)
        shUniform(float4, lightAmbient)                       @shAutoConstant(lightAmbient, ambient_light_colour)
        shUniform(float4, materialAmbient)                    @shAutoConstant(materialAmbient, surface_ambient_colour)
        shUniform(float4, materialDiffuse)                    @shAutoConstant(materialDiffuse, surface_diffuse_colour)
        shUniform(float4, materialSpecular)                   @shAutoConstant(materialSpecular, surface_specular_colour)
        shUniform(float4, materialEmissive)                   @shAutoConstant(materialEmissive, surface_emissive_colour)
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
        #if TERRAIN_LIGHT_MAP_TOGGLEABLE
        shUniform(float, enableTerrainLightMap)
        #endif
#endif

#if GRASS_WIND
        shUniform(float, grassFadeRange) @shSharedParameter(grassFadeRange)
#endif


#if SELECTED_GLOW
        shUniform(float, isSelected) @shAutoConstant(isSelected, custom, 1)
        shUniform(float, time) @shAutoConstant(time, time, 1)
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

float square(float x)
{
	return x * x;
}

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
        float depthPassthrough = UV.z;
#endif


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
    
        // shadows
#if SHADOWS
            float shadow = pssmDepthShadow (lightSpacePos0, invShadowmapSize0, shadowMap0, lightSpacePos1, invShadowmapSize1, shadowMap1, lightSpacePos2, invShadowmapSize2, shadowMap2, depthPassthrough, pssmSplitPoints);
#endif

#if SHADOWS
            float fadeRange = shadowFar_fadeStart.x - shadowFar_fadeStart.y;
            float fade = 1-((depthPassthrough - shadowFar_fadeStart.y) / fadeRange);
            shadow = (depthPassthrough > shadowFar_fadeStart.x) ? 1.0 : ((depthPassthrough > shadowFar_fadeStart.y) ? 1.0-((1.0-shadow)*fade) : shadow);
#endif

#if !(SHADOWS)
            float shadow = 1.0;
#endif


#if TERRAIN_LIGHT_MAP
		float shadowingLM;
		float2 worldPos = shMatrixMult(wMat, float4(objSpacePositionPassthrough.xyz, 1)).xz;
		float2 lmTexCoord = (worldPos / terrainWorldSize) + 0.5;
		shadowingLM = shSample(terrainLightMap, lmTexCoord).x;
        #if TERRAIN_LIGHT_MAP_TOGGLEABLE
		shadow = min(shadow, (enableTerrainLightMap == 1) ? shadowingLM : 1.0);
		#else
		shadow = min(shadow, shadowingLM);
		#endif
#endif


  float3 lightDir = normalize(lightPosObjSpace.xyz);

#if GRASS_WIND
  float n_dot_l = 1;
#else
	#if TWOSIDE_DIFFUSE
		float n_dot_l = abs(dot(normal, lightDir));
	#else
		float n_dot_l = max(dot(normal, lightDir), 0);
	#endif
#endif
    
  float3 eyeDir = normalize(camPosObjSpace.xyz - objSpacePositionPassthrough.xyz);
  float3 halfAngle = normalize (lightDir + eyeDir);
	
	float4 specTex = float4(1,1,1,1);

#if SPEC_MAP
	specTex = shSample(specMap, UV.xy);
	float ts = specTex.a * 255;
#else
	float ts = materialShininess;
#endif
	
#if TWOSIDE_DIFFUSE
	float n_dot_h = abs(dot(normal, halfAngle));
#else 
	float n_dot_h = max(dot(normal, halfAngle), 0);
#endif
	
	float specular_coeff = 0;
  
	if (n_dot_l > 0)
	{
		///////////////////////////////////////////
		//blinn-phong
		///////////////////////////////////////////
		/**/
#if BLINNPHONG
    specular_coeff = pow(n_dot_h, ts) / n_dot_l;
#endif
		/**/
    
    ///////////////////////////////////////////
    //cook torrance
    ///////////////////////////////////////////
    /**/
#if COOKTORRANCE
    float v_dot_h = dot(eyeDir, halfAngle);
		float n_dot_v = dot(normal, eyeDir);
		float one_over_n_dot_v = 1.0 / n_dot_v;
		
		//beckmann term
		float cm = beckmann_term; //tweakable
		float m = cm * cm;
		float t = n_dot_h * n_dot_h;
		float d = exp( (t - 1) / (m * t) ) / (m * t * t);
		
		//fresnel term
		float cf = fresnel_term; //tweakable
		float f = cf + ( 1 - cf ) * pow( 1 - v_dot_h, 5 );
		
		n_dot_h = n_dot_h + n_dot_h;
		
		//geometric term
		float g = 0;
		if(n_dot_v < n_dot_l)
		{
      if(n_dot_v * n_dot_h < v_dot_h)
			{
        g = n_dot_h / v_dot_h;
			}
			else
			{
        g = one_over_n_dot_v;
			}
		}
    else
		{
      if(n_dot_l * n_dot_h < v_dot_h)
			{
        g = n_dot_h * n_dot_l / (v_dot_h * n_dot_v);
			}
			else
			{
        g = one_over_n_dot_v;
			}
		}
		
		specular_coeff = (d * g * f) / n_dot_l;
#endif
		/**/
		
		///////////////////////////////////////////
    //ward
    ///////////////////////////////////////////
    /**/
#if WARD
		//tweakables
		float ax = aniso_term_x;
		float ay = aniso_term_y;
#if NORMAL_MAP
		float3 xx = tangentPassthrough.xyz;
		float3 yy = binormal;
#else
		//tweakables
		float3 xx = cross( normal, float3( 1, 0, 0 ) );
		float3 yy = cross( normal, float3( 0, 1, 0 ) );
#endif
		
		float exponent = -( square( dot( halfAngle, xx ) / ax ) + 
												square( dot( halfAngle, yy ) / ay ) ) / square( n_dot_h );
												
		specular_coeff = ( 1.0 / ( 4.0 * 3.14159265 * ax * ay * sqrt( n_dot_l * dot( eyeDir, normal ) ) ) ) * exp( exponent );
		n_dot_l /= 3.14159265;
#endif
		/**/
		
		///////////////////////////////////////////
    //ashikhman-shirley
    ///////////////////////////////////////////
    /**/
#if ASHIKHMANSHIRLEY
#if NORMAL_MAP
		float3 xx = tangentPassthrough.xyz;
		float3 yy = binormal;
#else
		//tweakables
		float3 xx = cross( normal, float3( 1, 0, 0 ) );
		float3 yy = cross( normal, float3( 0, 1, 0 ) );
#endif
		
		float h_dot_v = dot( halfAngle, eyeDir );
		float h_dot_x = dot( halfAngle, xx );
		float h_dot_y = dot( halfAngle, yy );
		float n_dot_v = dot( normal, eyeDir );
		
		//fresnel term
		float cf = fresnel_term; //tweakable
		float f = cf + ( 1 - cf ) * pow( 1 - h_dot_v, 5 );
		
		//tweakables
		float nu = aniso_term_x * 1000;
		float nv = aniso_term_y * 1000;
		float norm_s = sqrt( ( nu + 1 ) * nv ) / ( 8 * 3.14159265 );
		float n = ( nu * square( h_dot_x ) + nv * square( h_dot_y ) ) / ( 1 - square( n_dot_h ) );
		
		float rho_s = norm_s * f * pow( n_dot_h, n ) / ( h_dot_v * max( n_dot_v, n_dot_l ) );
		specular_coeff = rho_s;
		
		float rho_d = 28 / ( 23 * 3.14159265 ) * ( 1 - pow( 1 - n_dot_v / 2, 5 ) ) * ( 1 - pow( 1 - n_dot_l / 2, 5 ) );
		n_dot_l = rho_d;
#endif
		/**/
	}
	
	float3 diffuse_albedo = shOutputColour(0).xyz;
	float3 specular_albedo = specTex.xyz;
	
	float3 emissive_color = materialEmissive.xyz;
	float3 ambient_color = materialAmbient.xyz;
	float3 diffuse_color = materialDiffuse.xyz;
	float3 specular_color = materialSpecular.xyz;
	
	float3 light_ambient_color = lightAmbient.xyz;
	float3 light_diffuse_color = lightDiffuse.xyz;
	float3 light_specular_color = lightSpecular.xyz;

  shOutputColour(0).xyz =  ambient_color * diffuse_albedo * light_ambient_color + emissive_color +
														(diffuse_albedo * diffuse_color * light_diffuse_color * n_dot_l * diffuse_mul + 
														 specular_albedo * specular_color * light_specular_color * specular_coeff * spec_mul) * shadow;

#if ENV_MAP           
        float3 r = reflect( -eyeDir, normal );
        r = normalize(shMatrixMult(wMat, float4(r, 0)).xyz); 
        
        r.z = -r.z;
		float4 envColor = shCubicSample(envMap, r);
		
		float reflectionFactor = 1;
		
		#if REFL_MAP
		reflectionFactor *= shSample(reflMap, UV.xy).r;
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

        // alpha
        shOutputColour(0).a *= materialDiffuse.a;
        
#if ALPHA_MAP
        shOutputColour(0).a = shSample(alphaMap, float2(UV.x, UV.y * 0.01)).r;
#endif

#if SPECULAR_ALPHA
        // bump alpha with specular
        shOutputColour(0).a = min(shOutputColour(0).a + (specular_albedo * specular_color * light_specular_color * specular_coeff).x ,1);
#endif

#if GRASS_WIND
        // grass distance fading
        float dist = distance(camPosObjSpace.xz, objSpacePositionPassthrough.xz);
        shOutputColour(0).a *= (2.0f - (2.0f * dist / grassFadeRange));
#endif

#if SOFT_PARTICLES
        float2 screenUV = screenPosition.xy / screenPosition.z;
        screenUV = screenUV * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
        screenUV += (viewportSize.zw) * 0.5;
		screenUV.y =(1-shSaturate(flip))+flip*screenUV.y;
		float depthTex = shSample(sceneDepth, screenUV).x * far;
		float4 worldPos = shMatrixMult(wMat, float4(objSpacePositionPassthrough.xyz, 1));
		float distanceToPixel = length(worldPos.xyz - camPosWS.xyz);
		float thickness = 0.5;
		float tNear = distanceToPixel - thickness;
		float tFar = distanceToPixel + thickness;
        float depthAlpha = shSaturate(depthTex - distanceToPixel);
        shOutputColour(0).a *= depthAlpha;
#endif

#if SELECTED_GLOW
        shOutputColour(0).xyzw += isSelected * (float4(0.14, 0.22, 0.36, 0.36) * (0.5f + 0.1f * cos(3.f * time)));
#endif
		//shOutputColour(0).xyz = shOutputColour(0).xyz * 0.001 + normal.xyz;  // normal test

#if MRT
        shOutputColour(1) = float4(UV.w, normalize(viewNormal.xyz));
        shOutputColour(2) = float4(UV.z / far, 0, UV.z / objSpacePositionPassthrough.w, 0);
#endif

    }

#endif
