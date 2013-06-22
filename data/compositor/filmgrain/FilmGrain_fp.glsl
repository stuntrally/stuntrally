#version 120

varying vec2 uv;
uniform float time;
uniform vec4 grainparams;
uniform sampler2D frame;
uniform sampler3D noiseTex;

vec3 Overlay(vec3 a, vec3 b){
    return pow(abs(b.r), 2.2) < 0.5 ? 2 * a * b : 1.0 - 2 * (1.0 - a) * (1.0 - b);
}

void main(void)
{
	vec2 pixelSize = grainparams.xy;
	float noiseIntensity = grainparams.w;//grainparams.z;
	float exposure = 1;//grainparams.w;
	vec3 framecolor= texture2D (frame,uv).rgb;
	vec2 coord = uv * 2.0;
    coord.x *= pixelSize.y / pixelSize.x;
    float noise = texture3D(noiseTex, vec3(coord, time)).r;
    float exposureFactor = exposure / 2.0;
    exposureFactor = sqrt(exposureFactor);
    float t = mix(3.5 * noiseIntensity, 1.13 * noiseIntensity, exposureFactor);
    gl_FragColor = vec4(Overlay(framecolor, mix(vec3(0.5,0.5,0.5), vec3(noise,noise,noise), t)),1.0);
	
}

