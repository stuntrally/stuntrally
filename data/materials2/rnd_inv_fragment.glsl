#version 120

varying vec2 uv;
uniform sampler2D diffuseMap;

void main(void)
{
	vec2 uv2 = vec2(uv.y, 1.f - uv.x); // rotate 90'
	gl_FragColor = vec4(1,1,1,0) - texture2D(diffuseMap, uv2); // inverse
}
