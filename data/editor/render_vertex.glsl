#version 120

attribute vec4 vertex;
attribute vec2 uv0;
attribute vec4 colour;
attribute vec4 normal;

uniform mat4 wMat;
uniform mat4 wvpMat;

varying vec2 uv;
varying vec4 wp;
varying vec4 p;
varying vec4 n;
varying vec4 c;


void main(void)
{
	uv = uv0;
	wp = (wMat * vertex);
	gl_Position = (wvpMat * vertex);
	n = normal;
	c = colour;
}
