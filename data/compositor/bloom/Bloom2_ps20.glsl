//-------------------------------
//Bloom_ps20.glsl
// Blends using weights the blurred image with the sharp one
// Params:
//   OriginalImageWeight
//   BlurWeight
//-------------------------------

uniform sampler2D RT;
uniform sampler2D Blur1;

uniform float OriginalImageWeight;
uniform float BlurWeight;

varying vec2 uv;

void main()
{
    vec4 sharp;
    vec4 blur;
    
    sharp = texture2D( RT, uv);
    blur = texture2D( Blur1, uv);
    
    gl_FragColor = ( (blur * BlurWeight) + (sharp * OriginalImageWeight) );
    //gl_FragColor = vec4(0);
}
