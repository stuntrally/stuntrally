#version 120

varying vec2 uv;

uniform sampler2D diffuseMap;

void main(void)
{
	gl_FragColor = texture2D(diffuseMap, uv);
}
