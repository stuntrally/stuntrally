#version 120

attribute vec4 colour;
attribute vec2 uv0;
attribute vec4 vertex;

varying vec2 uv;
varying vec4 clr;

uniform mat4 wvpMat;

void main(void)
{
	uv = uv0;
	clr = colour;
	gl_Position = (wvpMat * vertex);
}
