#version 120

varying vec2 uv;
varying vec4 clr;

uniform float showTerrain;
uniform float showBorder;
uniform float square;

uniform sampler2D roadMap;
uniform sampler2D circleMap;
uniform sampler2D terMap;

void main(void)
{
	vec4 road = texture2D(roadMap, uv);
	vec4 cir = square > 0.5f ? vec4(1,0,1,1) : texture2D(circleMap, clr.xy);
	vec4 ter = texture2D(terMap, uv);

	vec3 cl, bcl = vec3(1.1,1.4,1.5);  //border color
	vec4 c;  float a,b;

	if (showBorder > 0.5f)
	{
		if (showTerrain > 0.5f)
		{
			a = cir.a;
			cl = mix(ter.rgb,road.rgb, road.a);
			c = vec4(mix(cir.g * bcl, cl.rgb, cir.r), a);
		}else
		{
			a = road.a * cir.a;  b = cir.a - cir.r*0.8f;  //par alpha clear
			cl = road.rgb;
			c = vec4(mix(cir.g * bcl, cl, cir.r), max(a, b));
		}
	}else
	{
		if (showTerrain > 0.5f)
		{
			a = cir.b;
			c = vec4(mix(ter.rgb,road.rgb, road.a), a);
		}else
		{
			a = cir.b;
			c = vec4(road.rgb, road.a * a);
		}
	}
	gl_FragColor = c;
}
