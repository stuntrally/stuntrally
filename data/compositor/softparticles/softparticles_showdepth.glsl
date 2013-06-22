#version 120

varying vec2 uv;
uniform sampler2D tex1;
uniform float far;

void main(void)
{

	vec4 color=texture2D(tex1, uv);
	color=vec4(color.x*far/20,color.x*far/20,color.x*far/20,1);
color=sqrt(color);
//	test -only show particles
	gl_FragColor = color;
}
