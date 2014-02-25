#include "hdrutils.hlsl"

/* Downsample a 2x2 area and convert to greyscale
*/
float4 downscale2x2Luminence(
	float2 uv : TEXCOORD0,
	uniform float2 texelSize, // depends on size of source texture
	uniform sampler2D inRTT : register(s0)
    ) : COLOR
{
	float4 accum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float2 texOffset[4] = {
		-0.5, -0.5,
		-0.5,  0.5, 
		 0.5, -0.5,
		 0.5, 0.5 };

	for( int i = 0; i < 4; i++ )
	{
		// Get colour from source
		accum += tex2D(inRTT, uv + texelSize * texOffset[i]);
	}

	// Adjust the accumulated amount by lum factor
	// Cannot use float3's here because it generates dependent texture errors because of swizzle
	float lum = dot(accum, LUMINENCE_FACTOR);
	// take average of 4 samples
	lum *= 0.25;
	return float4(lum, lum, lum, lum);
}

/* Downsample a 3x3 area 
 * This shader is used multiple times on different source sizes, so texel size has to be configurable
*/
float4 downscale3x3(
	float2 uv : TEXCOORD0,
	uniform float2 texelSize, // depends on size of source texture
	uniform sampler2D inRTT : register(s0)
    ) : COLOR
{
	float4 accum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float2 texOffset[9] = {
		-1.0, -1.0,
		 0.0, -1.0,
		 1.0, -1.0,
		-1.0,  0.0,
		 0.0,  0.0,
		 1.0,  0.0,
		-1.0,  1.0,
		 0.0,  1.0,
		 1.0,  1.0
	};

	for( int i = 0; i < 9; i++ )
	{
		// Get colour from source
		accum += tex2D(inRTT, uv + texelSize * texOffset[i]);
	}
    
	// take average of 9 samples
	accum *= 0.1111111111111111;
	return accum;
}

/* Adeptive downsample a 3x3 area 
 * This shader is used multiple times on different source sizes, so texel size has to be configurable
*/
float4 downscale3x3Adept(
	float2 uv : TEXCOORD0,
	uniform float2 texelSize, // depends on size of source texture
	uniform float dTime,
	uniform float adaptationScale,
	uniform sampler2D inRTT : register(s0),
	uniform sampler2D oldRTT : register(s1)
	) : COLOR
{
	float4 accum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float2 texOffset[9] = {
		-1.0, -1.0,
		 0.0, -1.0,
		 1.0, -1.0,
		-1.0,  0.0,
		 0.0,  0.0,
		 1.0,  0.0,
		-1.0,  1.0,
		 0.0,  1.0,
		 1.0,  1.0
	};

	for( int i = 0; i < 9; i++ )
	{
		// Get colour from source
		accum += tex2D(inRTT, uv + texelSize * texOffset[i]);
	}

	// take average of 9 samples
	accum *= 0.1111111111111111;

	float4 oldValue = tex2D(oldRTT, float2(0.5f, 0.5f));

	//determin if rods or cones are active
	//Perceptual Effects in Real-time Tone Mapping: Equ(7)    
	float sigma = saturate(0.4 / (0.04 + accum.x));

	//interpolate tau from taurod and taucone depending on lum
	//Perceptual Effects in Real-time Tone Mapping: Equ(12)
	float Tau = lerp(0.01, 0.04, sigma) * adaptationScale;
	float blendFactor = 1.0f - exp(-(dTime)/Tau);

	//calculate adaption
	//Perceptual Effects in Real-time Tone Mapping: Equ(5)
	return lerp(oldValue, accum, 1 - exp(-(dTime)/Tau));
}

float4 copySample(
	float2 uv : TEXCOORD0,
	uniform sampler2D sample : register(s0)
	) : COLOR
{
	return max(float4(0,0,0,0), tex2D(sample, uv));
}

/* Downsample a 3x3 area from main RTT and perform a brightness pass
*/
float4 downscale3x3brightpass(
	float2 uv : TEXCOORD0,
	uniform float2 texelSize, // depends on size of source texture
	uniform float4 toneMapSettings,
	uniform float4 bloomSettings,
	uniform sampler2D inRTT : register(s0),
	uniform sampler2D inLum : register(s1)
    ) : COLOR
{
	float4 accum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float2 texOffset[9] = {
		-1.0, -1.0,
		 0.0, -1.0,
		 1.0, -1.0,
		-1.0,  0.0,
		 0.0,  0.0,
		 1.0,  0.0,
		-1.0,  1.0,
		 0.0,  1.0,
		 1.0,  1.0
	};

	for( int i = 0; i < 9; i++ )
	{
		// Get colour from source
		accum += tex2D(inRTT, uv + texelSize * texOffset[i]);
	}
    
	// take average of 9 samples
	accum *= 0.1111111111111111;

	// Sample the luminence texture
	float4 lum = tex2D(inLum, float2(0.5f, 0.5f));

	// Tone map result
//	accum = toneMap(accum, lum.r, toneMapSettings);

	// Reduce bright and clamp
	accum = max(float4(0.f,0.f,0.f,0.f), accum -  bloomSettings.x);//
//	accum /= (bloomSettings.x + accum);
//	accum /= (1.0 + accum);

	return accum;
}

/* Gaussian bloom, requires offsets and weights to be provided externally
*/
float4 bloom(
		float2 uv : TEXCOORD0,
		uniform float4 sampleWeights[15],	
		uniform float4 sampleOffsets[15],
		uniform sampler2D inRTT : register(s0)
		) : COLOR
{
	float4 accum = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float2 sampleUV;

	for( int i = 0; i < 15; i++ )
	{
		// Sample from adjacent points, 7 each side and central
		sampleUV = uv + sampleOffsets[i];
		accum += sampleWeights[i] * tex2D(inRTT, sampleUV);
	}

	return float4(accum.rgb,1.0);

}


/* Final scene composition, with tone mapping
*/
float4 finalToneMapping(
	float2 uv : TEXCOORD0,
	uniform float4 toneMapSettings,
	uniform float4 bloomSettings,
	uniform float4 vignettingSettings,
    uniform sampler2D inRTT : register(s0),
	uniform sampler2D inBloom : register(s1),
	uniform sampler2D inLum : register(s2)
	) : COLOR
{
	// Get main scene colour
	float4 sceneCol = tex2D(inRTT, uv);

	// Get luminence value
	float4 lum = tex2D(inLum, float2(0.5f, 0.5f));

	// tone map this
	float4 toneMappedSceneCol = toneMap(sceneCol, lum.r, toneMapSettings);

	// Get bloom colour
	float4 bloom = tex2D(inBloom, uv);
	float3 bloomAdjusted = bloom.rgb * bloomSettings.y;
	// Add scene & bloom
	float4 finalColor = float4(toneMappedSceneCol.rgb + clamp(bloomAdjusted,float3(0,0,0),float3(1,1,1)), 1.0f);


	float Radius = vignettingSettings.x;
	float Darkness= vignettingSettings.y;

	float2 inTex = uv - 0.5;
	float vignette  = 1 - dot(inTex, inTex);
	finalColor.rgb    *= saturate(pow(vignette, Radius) + Darkness);

/*
	float ChromaticAberration=vignettingSettings.z;
	float SaturationLoss=vignettingSettings.w;
    float2 inTex = iTexCoord - float2(0.5, 0.5);
    float dot_tex = dot(inTex, inTex);
    float vignette = 1 - dot_tex;

   vignette = saturate(pow(vignette, Radius) + EPF_Darkness);
   
   // chromatic aberration
   // compute green and blue sampling offset
   float2 sign_tc = sign(inTex);
   float vignette_inv =  1 - vignette;
   float2 g_tc = iTexCoord - ChromaticAberration * inTex * vignette_inv;
   float2 b_tc = iTexCoord - ChromaticAberration * 2.0f * inTex * vignette_inv;
   // sample again green and blue
   float base_g = tex2D(RT, g_tc).g;
   float base_b = tex2D(inRTT, b_tc).b;
   
   // compose result color
   float3 result = float3(finalColor.r, base_g, base_b);
   
   // apply vignette as color multiplication
   result *= vignette;
   
   // correct saturation
   // convert to hsv
   float3 result_hsv = RGBtoHSV(result);
   result_hsv.y -= SaturationLoss * vignette_inv;
   result_hsv.y = saturate(result_hsv.y);
   result = HSVtoRGB(result_hsv);
  */ 
	return finalColor;
}


