#version 120

attribute vec4 vertex;
attribute vec2 uv0;
varying vec2 uv;
uniform mat4 wvp;

void main(void)
{
	gl_Position = (wvp * vertex);
	uv = uv0;
}
