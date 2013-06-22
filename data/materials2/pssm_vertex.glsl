#version 120

attribute vec4 vertex;
attribute vec2 uv0;

varying vec2 uv;
varying vec2 depth;

uniform mat4 wvpMat;

void main(void)
{
	gl_Position = (wvpMat * vertex);

	depth.x = gl_Position.z;
	depth.y = gl_Position.w;

	uv = uv0;
}
