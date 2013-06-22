#version 120

varying vec2 uv;

uniform sampler2D RT;
uniform sampler2D Sum;

uniform float blur;

void main(void)
{
   vec4 render = texture2D(RT, uv);
   vec4 sum = texture2D(Sum, uv);

   gl_FragColor = mix(render, sum, blur);
}
