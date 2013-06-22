#version 120

varying vec2 uv;
uniform sampler2D tex1;


void main(void)
{
	vec4 scene = texture2D(tex1, uv);
	gl_FragColor = scene;
}
