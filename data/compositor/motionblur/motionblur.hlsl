
struct VS_OUTPUT {
   float4 Pos: POSITION;
   float2 uv: TEXCOORD0;
   float3 ray : TEXCOORD1;
};

VS_OUTPUT motionblur_vs(
    float4 Pos: POSITION,
    uniform float3 farCorner
)
{
   VS_OUTPUT Out;

   // Clean up inaccuracies
   Pos.xy = sign(Pos.xy);

   Out.Pos = float4(Pos.xy, 0, 1);
   // Image-space
   Out.uv.x = 0.5 * (1 + Pos.x);
   Out.uv.y = 0.5 * (1 - Pos.y);

   Out.ray = farCorner * float3(Out.uv * 2.0 - 1.0, 1.0);

   return Out;
}

float4 motionblur_ps(
   float2 uv: TEXCOORD0,
   float3 ray : TEXCOORD1,

   uniform sampler2D scene : TEXUNIT0,
   uniform sampler2D depthTex : TEXUNIT1,
   uniform sampler2D maskTex : TEXUNIT2,
    
   uniform float4x4 invViewMat,
   uniform float4x4 prevViewProjMat,
   uniform float fps,
   uniform float far,
   uniform float intensity
   
) : COLOR
{
    const float nSamples = 32;
    const float targetFps = 60;

    float depth = tex2D(depthTex, uv).x; // linear depth
    // IN.ray will be distorted slightly due to interpolation
    // it should be normalized here
    float3 viewPos = normalize(ray) * depth * far;

    float3 worldPos = (mul(invViewMat, float4(viewPos,1.0))).xyz;

   // get previous screen space position
    float4 previous = mul(prevViewProjMat, float4(worldPos, 1.0));
    previous.xyz /= previous.w;
    previous.xy = previous.xy * 0.5 + 0.5;

    // account for render target flipping done by Ogre in GL mode, this line would be removed for a D3D port
    //previous.y = 1-previous.y;

    float2 blurVec = previous.xy - uv;

    blurVec *= intensity * fps / targetFps;

    // perform blur
    float4 orig = tex2D(scene, uv);
    float4 result = orig;
    float4 centerMask = tex2D(maskTex, uv);
    for (int i = 1; i < nSamples; ++i) {

        // get offset in range [-0.5, 0.5]
        float2 offset = blurVec * (float(i) / float(nSamples - 1) - 0.5) * centerMask.w;

        float4 mask = tex2D(maskTex, uv + offset);

        // sample & add to result
        // to prevent ghosting
        result += lerp(orig, tex2D(scene, uv + offset), mask.w);
    }

    result /= float(nSamples);

    return result;
}
