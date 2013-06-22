#version 120

attribute vec3 normal;
attribute vec2 uv0;
attribute vec4 vertex;

varying vec2 uv;
varying vec3 ray;

uniform mat4 wvp;
uniform vec3 farCorner;

void main(void)
{
	uv = uv0;
	// calculate the correct ray (modify XY parameters based on screen-space quad XY)	
	ray = farCorner * vec3(uv0*2.0-1.0 , 1);
	ray.y *= -1;
	gl_Position = (wvp * vertex);
}
