#version 120

varying vec2 uv;
uniform sampler2D diffuseAdd;
uniform sampler2D diffuseMap;

void main(void)
{
	vec4 rd = texture2D(diffuseAdd, uv);
	vec4 ter = texture2D(diffuseMap, uv);
	gl_FragColor = vec4(mix(rd.rgb, ter.rgb, 1-rd.a), 1);
}
