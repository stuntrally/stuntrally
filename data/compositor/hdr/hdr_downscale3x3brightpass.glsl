#version 120

uniform sampler2D inRTT;
uniform sampler2D inLum;
uniform vec2 texelSize;
uniform vec4 bloomSettings;


varying vec2 uv;
const vec4 BRIGHT_LIMITER = vec4(0.6, 0.6, 0.6, 0.0);

// declare external function
vec4 toneMap(in vec4 inColour, in float lum);

void main(void)
{
	vec4 accum = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	vec2 texOffset[9] = vec2[9] (
		vec2(-1.0, -1.0),
		 vec2(0.0, -1.0),
		 vec2(1.0, -1.0),
		vec2(-1.0,  0.0),
		 vec2(0.0,  0.0),
		 vec2(1.0,  0.0),
		vec2(-1.0,  1.0),
		 vec2(0.0,  1.0),
		 vec2(1.0,  1.0)
	);

	for( int i = 0; i < 9; i++ )
	{
		// Get colour from source
		accum += texture2D(inRTT, uv + texelSize * texOffset[i]);
	}
    
	// take average of 9 samples
	accum *= 0.1111111111111111;

	// Sample the luminence texture
	vec4 lum = texture2D(inLum, vec2(0.5f, 0.5f));

	// Tone map result
//	accum = toneMap(accum, lum.r, toneMapSettings);

	// Reduce bright and clamp
	accum = max(vec4(0.0f), accum -  bloomSettings.x);//
//	accum /= (bloomSettings.x + accum);
//	accum /= (1.0 + accum);

	gl_FragColor = accum;

}
