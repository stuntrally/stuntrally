#version 120

varying vec2 uv;
varying vec3 ray;

uniform sampler2D scene;
uniform sampler2D depthTex;
uniform sampler2D maskTex;

uniform float far;

uniform mat4 invViewMat;
uniform mat4 prevViewProjMat;

uniform float fps;
uniform float intensity;

const float nSamples = 32.0;
const float targetFps = 24.0; // 60 * default intensity(0.4)

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

    // account for render target flipping done by Ogre in GL mode, this line would be removed for a D3D port
    previous.y = 1-previous.y;

    vec2 blurVec = previous.xy - uv;

    blurVec *= intensity * fps / targetFps;

    // perform blur
    vec4 orig = texture2D(scene, uv);
    vec4 result = orig;
    vec4 centerMask = texture2D(maskTex, uv);
    for (int i = 1; i < nSamples; ++i) {

        // get offset in range [-0.5, 0.5]
        vec2 offset = blurVec * (float(i) / float(nSamples - 1) - 0.5) * centerMask.w;

        vec4 mask = texture2D(maskTex, uv + offset);

        // sample & add to result
        // to prevent ghosting
        result += mix(orig, texture2D(scene, uv + offset), mask.w);
    }

    result /= float(nSamples);


    gl_FragColor = result;
}
