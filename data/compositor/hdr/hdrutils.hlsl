// RGBE mode utilities
// RGB each carry a mantissa, A carries a shared exponent
// The exponent is calculated based on the largest colour channel


float3 decodeRGBE8(in float4 rgbe)
{
	// get exponent (-128 since it can be +ve or -ve)
	float exp = rgbe.a * 255 - 128;

	// expand out the rgb value
	return rgbe.rgb * exp2(exp);
}

float4 encodeRGBE8(in float3 rgb)
{
	float4 ret;

    // What is the largest colour channel?
	float highVal = max(rgb.r, max(rgb.g, rgb.b));
	
	// Take the logarithm, clamp it to a whole value
	float exp = ceil(log2(highVal));

    // Divide the components by the shared exponent
	ret.rgb = rgb / exp2(exp);
	
	// Store the shared exponent in the alpha channel
	ret.a = (exp + 128) / 255;

	return ret;
}


//static const float4 LUMINENCE_FACTOR  = float4(0.27f, 0.67f, 0.06f, 0.0f);
static const float4 LUMINENCE_FACTOR  = float4(0.2126f, 0.7152f, 0.0722f, 0.0f);
static const float MIDDLE_GREY = 0.5f;
static const float FUDGE = 0.001f;
static const float L_WHITE = 2.5f;
//static const float4 BRIGHT_LIMITER = float4(0.6f, 0.6f, 0.6f, 0.0f);
static const float4 BRIGHT_LIMITER = float4(1.0f, 1.0f, 1.0f, 0.0f);
static const float4 BRIGHT_OFFSET = float4(0.5f, 0.5f, 0.5f, 0.0f);

//static const float MIN_KEY = 0.25f;
//static const float MAX_KEY = 1.5f;

/** Tone mapping function 
@note Only affects rgb, not a
@param inColour The HDR colour
@param lum The scene lumninence 
@returns Tone mapped colour
*/
float4 toneMap(float4 inColour, float lum, float4 toneMapSettings)
{
//	toneMapSettings = float4(0.8,.4,0,toneMapSettings.x);
	// From Reinhard et al
	// "Photographic Tone Reproduction for Digital Images"

	// Initial luminence scaling (equation 2)
	float key = saturate(toneMapSettings.z - toneMapSettings.z / (lum * toneMapSettings.y + 1)) + toneMapSettings.y;

	float4 scaledlum = inColour * key / (FUDGE + lum);
	inColour.rgb = scaledlum * (1 + scaledlum / (toneMapSettings.x * toneMapSettings.x)) / (0.5 + scaledlum);

	return inColour;
	
}

