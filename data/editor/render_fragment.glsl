#version 120

uniform vec3 ambient;
uniform vec4 matDif;

varying vec2 uv;
varying vec4 wp;
varying vec4 n;
varying vec4 c;


void main(void)
{
	float bridge = c.x, pipe = c.y;
	float norm = abs(n.y);  //abs

	float pwr = mix(mix(8, 8, bridge), 4, pipe);
	float ter = mix(mix(1, 0, bridge), 0, pipe);
	float diffuse = 1 - mix( 1-mix(pow(norm, pwr), pow(norm, pwr), pipe),
		pow(1.f - 2.f*acos(norm)/3.141592654, pwr), ter);
	vec3 clrLi = ambient + diffuse * matDif.rgb;
 
	//return vec4(IN.n, 1);  // test
	//return IN.c;  // test
	//float alpha = mix(1, 253.f/255.f, bridge);

	vec3 clr = mix( mix(
		vec3(1, 1, 1) * clrLi,
		vec3(0, 0.8,1) * (0.4+0.7*clrLi), bridge),
		vec3(1,0.8, 0) * (0.2+1.0*clrLi), pipe);
	gl_FragColor = vec4(clr, 1);
}
