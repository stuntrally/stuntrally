#version 120

attribute vec2 uv0;
attribute vec4 vertex;

varying vec2 uv;

uniform mat4 wvp;

void main(void)
{
	uv = uv0;
	gl_Position = (wvp * vertex);
}
