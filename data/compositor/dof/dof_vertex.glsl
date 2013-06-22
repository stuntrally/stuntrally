#version 120

attribute vec4 vertex;
uniform mat4  wvp;

attribute vec2 uv0;
varying vec2 UV;

void main(void)
{
	gl_Position = (wvp * vertex);
	UV = uv0;
}
