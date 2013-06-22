#version 120

varying vec2 uv;
varying vec2 depth;

uniform vec4 pssmSplitPoints;

void main(void)
{
	float finalDepth = depth.x / depth.y;
	
	gl_FragColor = vec4(finalDepth, finalDepth, finalDepth, 1);
}
