#version 120

varying vec2 uv;
uniform sampler2D tex1;
uniform sampler2D tex2;

void main(void)
{
	gl_FragColor = texture2D(tex1, uv) * texture2D(tex2, uv);
}
