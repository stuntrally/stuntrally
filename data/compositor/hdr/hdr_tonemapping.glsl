#version 120

/* Final scene composition, with tone mapping
*/

varying vec2 uv;
uniform vec4 toneMapSettings;
uniform vec4 bloomSettings;
uniform vec4 vignettingSettings;

uniform sampler2D inRTT;
uniform sampler2D inBloom;
uniform sampler2D inLum;

#define FUDGE 0.001f

/** Tone mapping function 
@note Only affects rgb, not a
@param inColour The HDR colour
@param lum The scene lumninence 
@returns Tone mapped colour
*/
vec4 toneMap(vec4 inColour, float lum, vec4 toneMapSettings)
{
//	toneMapSettings = vec4(0.8,.4,0,toneMapSettings.x);
	// From Reinhard et al
	// "Photographic Tone Reproduction for Digital Images"

	// Initial luminence scaling (equation 2)
	float key = clamp(toneMapSettings.z - toneMapSettings.z / (lum * toneMapSettings.y + 1), 0, 1) + toneMapSettings.y;

	vec4 scaledlum = inColour * key / (FUDGE + lum);
	inColour.rgb = (scaledlum * (1 + scaledlum / (toneMapSettings.x * toneMapSettings.x)) / (0.5 + scaledlum)).rgb;

	return inColour;
	
}

void main(void)
{
	// Get main scene colour
	vec4 sceneCol = texture2D(inRTT, uv);

	// Get luminence value
	vec4 lum = texture2D(inLum, vec2(0.5f, 0.5f));

	// tone map this
	vec4 toneMappedSceneCol = toneMap(sceneCol, lum.r, toneMapSettings);

	// Get bloom colour
	vec4 bloom = texture2D(inBloom, uv);
	vec3 bloomAdjusted = bloom.rgb * bloomSettings.y;
	// Add scene & bloom
	vec4 finalColor = vec4(toneMappedSceneCol.rgb + clamp(bloomAdjusted,vec3(0,0,0),vec3(1,1,1)), 1.0f);


	float Radius = vignettingSettings.x;
	float Darkness= vignettingSettings.y;

	vec2 inTex = uv - 0.5;
	float vignette  = 1 - dot(inTex, inTex);
	finalColor.rgb    *= clamp(pow(vignette, Radius) + Darkness, 0, 1);

/*
	float ChromaticAberration=vignettingSettings.z;
	float SaturationLoss=vignettingSettings.w;
    vec2 inTex = iTexCoord - vec2(0.5, 0.5);
    float dot_tex = dot(inTex, inTex);
    float vignette = 1 - dot_tex;

   vignette = saturate(pow(vignette, Radius) + EPF_Darkness);
   
   // chromatic aberration
   // compute green and blue sampling offset
   vec2 sign_tc = sign(inTex);
   float vignette_inv =  1 - vignette;
   vec2 g_tc = iTexCoord - ChromaticAberration * inTex * vignette_inv;
   vec2 b_tc = iTexCoord - ChromaticAberration * 2.0f * inTex * vignette_inv;
   // sample again green and blue
   float base_g = texture2D(RT, g_tc).g;
   float base_b = texture2D(inRTT, b_tc).b;
   
   // compose result color
   vec3 result = vec3(finalColor.r, base_g, base_b);
   
   // apply vignette as color multiplication
   result *= vignette;
   
   // correct saturation
   // convert to hsv
   vec3 result_hsv = RGBtoHSV(result);
   result_hsv.y -= SaturationLoss * vignette_inv;
   result_hsv.y = saturate(result_hsv.y);
   result = HSVtoRGB(result_hsv);
  */ 
	gl_FragColor = finalColor;
}
