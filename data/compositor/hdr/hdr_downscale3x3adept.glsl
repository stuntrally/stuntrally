#version 120

/* Adeptive downsample a 3x3 area 
 * This shader is used multiple times on different source sizes, so texel size has to be configurable
*/

varying vec2 uv;
uniform vec2 texelSize;
uniform float dTime;
uniform float adaptationScale;
uniform sampler2D inRTT;
uniform sampler2D oldRTT;

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

	vec4 oldValue = texture2D(oldRTT, vec2(0.5f, 0.5f));

	//determin if rods or cones are active
	//Perceptual Effects in Real-time Tone Mapping: Equ(7)    
	float sigma = clamp((0.4 / (0.04 + accum.x), 0, 1));

	//interpolate tau from taurod and taucone depending on lum
	//Perceptual Effects in Real-time Tone Mapping: Equ(12)
	float Tau = lerp(0.01, 0.04, sigma) * adaptationScale;
	float blendFactor = 1.0f - exp(-(dTime)/Tau);

	//calculate adaption
	//Perceptual Effects in Real-time Tone Mapping: Equ(5)
	gl_FragColor = mix(oldValue, accum, 1 - exp(-(dTime)/Tau));
}
