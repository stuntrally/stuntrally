#version 120

varying vec2 uv;
varying vec4 wp;
varying vec4 n;
varying vec4 c;

void main(void)
{
	float bridge = c.w;

	gl_FragColor = bridge * vec4(1,1,1,1);
}
