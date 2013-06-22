#version 120

varying vec2 uv;
varying vec2 depth;

uniform vec4 pssmSplitPoints;
uniform sampler2D alphaMap;

void main(void)
{
	float finalDepth = depth.x / depth.y;
	vec4 trans = texture2D(alphaMap, uv);

	if (trans.w <= 0.5)
		discard;

	gl_FragColor = vec4(finalDepth, finalDepth, finalDepth, trans.w);
}
