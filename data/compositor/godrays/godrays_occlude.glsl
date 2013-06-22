#version 120

varying vec2 uv;
varying vec2 lightPos;
uniform sampler2D frame;
uniform float enableEffect;

void main(void)
{
	const float density = 0.2;
	const int samples = 16;
	const float weight = 0.8;
	const float decay = 1.05;
	
	vec2 deltaTexCoord = (uv - lightPos);
	
	deltaTexCoord *= 1.0f / samples * density;
	
	//the MRT green channel include the luminance info
	vec3 raysColor =vec3(1.0,1.0,1.0);
	vec3 col = texture2D(frame, uv).g *raysColor;
	float illuminationDecay = 1.0f;

	vec2 uv2 = uv;
	
	for (int i = 0; i < samples; i++) {
		uv2 -= deltaTexCoord;
		vec3 sample = texture2D(frame, uv2).g*raysColor;
		sample *= illuminationDecay * weight;
		col += sample;
		illuminationDecay *= decay;
	}
	
	gl_FragColor = vec4(col * 0.04, 1) *enableEffect;
}
