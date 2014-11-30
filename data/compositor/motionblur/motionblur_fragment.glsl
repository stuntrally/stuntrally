#version 120

varying vec2 uv;
varying vec3 ray;

uniform sampler2D scene;
uniform sampler2D depthTex;

uniform float far;

uniform mat4 invViewMat;
uniform mat4 prevViewProjMat;

uniform float textureFlipping;

uniform float fps;

const float nSamples = 32;
const float targetFps = 60;

void main(void)
{
    float depth = texture2D(depthTex, uv).x; // linear depth
    // IN.ray will be distorted slightly due to interpolation
    // it should be normalized here
    vec3 viewPos = normalize(ray) * depth * far;

    vec3 worldPos = (invViewMat * vec4(viewPos,1.0)).xyz;

   // get previous screen space position
    vec4 previous = prevViewProjMat * vec4(worldPos, 1.0);
    previous.xyz /= previous.w;
    previous.xy = previous.xy * 0.5 + 0.5;

    // account for render target flipping done by Ogre through the projection matrix
    previous.y = (1-clamp(textureFlipping, 0.0, 1.0))+textureFlipping*previous.y;

    vec2 blurVec = previous.xy - uv;

    blurVec *= fps / targetFps;

    // perform blur
    vec4 result = texture2D(scene, uv);
    for (int i = 1; i < nSamples; ++i) {
	// get offset in range [-0.5, 0.5]
	vec2 offset = blurVec * (float(i) / float(nSamples - 1) - 0.5);

	// sample & add to result
	result += texture2D(scene, uv + offset);
    }

    result /= float(nSamples);


    gl_FragColor = result;
    //gl_FragColor = vec4(previous.xy, 0.0, 1.0
}
