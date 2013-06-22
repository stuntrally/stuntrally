#version 120

varying vec2 uv;
uniform sampler2D sample;

void main(void)
{
	gl_FragColor = max(vec4(0,0,0,0), texture2D(sample, uv));
}
