struct VIn
{
    float4 p   : POSITION;
    float3 n   : NORMAL;
    float2 uv  : TEXCOORD0;
};

struct VOut
{
    float4 p   : POSITION;
    float2 uv  : TEXCOORD0;
    float3 ray : TEXCOORD1;
};

struct PIn
{
	float4 p   : POSITION; 
    float2 uv  : TEXCOORD0;
    float3 ray : TEXCOORD1;
};

VOut ssao_vs(VIn IN, uniform float4x4 wvp, uniform float3 farCorner)
{
    VOut OUT;
    OUT.p = mul(wvp, IN.p);
    // clean up inaccuracies for the UV coords
    float2 uv = sign(IN.p);
    // convert to image space
    uv = (float2(uv.x, -uv.y) + 1.0) * 0.5;
    OUT.uv = uv;
    // calculate the correct ray (modify XY parameters based on screen-space quad XY)
    OUT.ray = farCorner * float3(sign(IN.p.xy), 1);
    return OUT;
}

float3 computeZ(float2 xy)
{
    return float3(xy, sqrt(1.0 - dot(xy, xy)));
}

float4 ssao_ps(
    PIn IN,
    uniform float4x4 ptMat,
    uniform float far,
	uniform float4 fogParams,
    uniform sampler2D geomMap : TEXUNIT0,
    uniform sampler2D randMap  : TEXUNIT1): COLOR0
{
    #define MAX_RAND_SAMPLES 14

    const float3 RAND_SAMPLES[MAX_RAND_SAMPLES] =
    {
        float3(1, 0, 0),
        float3(	-1, 0, 0),
        float3(0, 1, 0),
        float3(0, -1, 0),
        float3(0, 0, 1),
        float3(0, 0, -1),
        normalize(float3(1, 1, 1)),
        normalize(float3(-1, 1, 1)),
        normalize(float3(1, -1, 1)),
        normalize(float3(1, 1, -1)),
        normalize(float3(-1, -1, 1)),
        normalize(float3(-1, 1, -1)),
        normalize(float3(1, -1, -1)),
        normalize(float3(-1, -1, -1))
    };

    // constant expressin != const int :(
    #define NUM_BASE_SAMPLES 6

    // random normal lookup from a texture and expand to [-1..1]
    float3 randN = tex2D(randMap, IN.uv * 24).xyz * 2.0 - 1.0;
    float4 geom = tex2D(geomMap, IN.uv);
    float depth = geom.x;

	float fogAmount = saturate(fogParams.x * (depth*far - fogParams.y) * fogParams.w);

    // IN.ray will be distorted slightly due to interpolation
    // it should be normalized here
    float3 viewPos = IN.ray * depth;

    // by computing Z manually, we lose some accuracy under extreme angles
    // considering this is just for bias, this loss is acceptable
    float3 viewNorm = geom.yzw;//computeZ(geom.yz);

    // accumulated occlusion factor
    float occ = 0;
    for (int i = 0; i < NUM_BASE_SAMPLES; ++i)
    {
        // reflected direction to move in for the sphere
        // (based on random samples and a random texture sample)
        // bias the random direction away from the normal
        // this tends to minimize self occlusion
        float3 randomDir = reflect(RAND_SAMPLES[i], randN) + viewNorm;

        // move new view-space position back into texture space
        #define RADIUS 0.2125
        float4 nuv = mul(ptMat, float4(viewPos.xyz + randomDir * RADIUS, 1));
        nuv.xy /= nuv.w;

        // compute occlusion based on the (scaled) Z difference
        float zd = saturate(far * (depth - tex2D(geomMap, nuv.xy).x));
        // this is a sample occlusion function, you can always play with
        // other ones, like 1.0 / (1.0 + zd * zd) and stuff
        occ += saturate(pow(1.0 - zd, 11) + zd);
    }
    occ /= NUM_BASE_SAMPLES;

    return float4( lerp( float3(occ, occ, occ), float3(1,1,1), fogAmount), 1);
}

void ssaoBlur_vs(in float4 position : POSITION
	,in float2 uv :TEXCOORD0
	,uniform float4x4 wvp
	,out float4 oPosition : POSITION
	,out float2 oUV :TEXCOORD0)
{
	oPosition = mul(wvp, position);
	oUV = uv;
	return;
}
#define NUM_BLUR_SAMPLES 8

float4 ssaoBlurX_ps(float4 iPosition : POSITION,float2 uv : TEXCOORD0,
    uniform float4 invTexSize,
    uniform sampler2D map : TEXUNIT0, uniform sampler2D geomMap : TEXUNIT1): COLOR0
{
//    return tex2D(ssaoMap, uv);
    float2 o = float2(invTexSize.x, 0);
    float4 sum = tex2D(map, uv) * (NUM_BLUR_SAMPLES + 1);
    float denom = NUM_BLUR_SAMPLES + 1;
    float4 geom = tex2D(geomMap, uv);
    for (int i = 1; i <= NUM_BLUR_SAMPLES; ++i)
    {
        float2 nuv = uv + o * i;
        float4 nGeom = tex2D(geomMap, nuv);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * (dot(geom.yzw, nGeom.yzw) > 0.9);
        sum += tex2D(map, nuv) * coef;
        denom += coef;
    }
    for (int i = 1; i <= 4; ++i)
    {
        float2 nuv = uv + o * -i;
        float4 nGeom = tex2D(geomMap, nuv);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * (dot(geom.yzw, nGeom.yzw) > 0.9);
        sum += tex2D(map, nuv) * coef;
        denom += coef;
    }
    return sum / denom;
}

float4 ssaoBlurY_ps(float4 iPosition : POSITION,float2 uv : TEXCOORD0,
    uniform float4 invTexSize,
    uniform sampler2D map : TEXUNIT0, uniform sampler2D geomMap : TEXUNIT1): COLOR0
{
//    return tex2D(map, uv);
    float2 o = float2(0, invTexSize.y);
    float4 sum = tex2D(map, uv) * (NUM_BLUR_SAMPLES + 1);
    float denom = NUM_BLUR_SAMPLES + 1;
    float4 geom = tex2D(geomMap, uv);
    for (int i = 1; i <= NUM_BLUR_SAMPLES; ++i)
    {
        float2 nuv = uv + o * i;
        float4 nGeom = tex2D(geomMap, nuv);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * (dot(geom.yzw, nGeom.yzw) > 0.9);
        sum += tex2D(map, nuv) * coef;
        denom += coef;
    }
    for (int i = 1; i <= 4; ++i)
    {
        float2 nuv = uv + o * -i;
        float4 nGeom = tex2D(geomMap, nuv);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * (dot(geom.yzw, nGeom.yzw) > 0.9);
        sum += tex2D(map, nuv) * coef;
        denom += coef;
    }
    return sum / denom;
}

void ssaoMultiply_vs(in float4 position : POSITION
	,in float2 uv :TEXCOORD0
	,uniform float4x4 wvp
	,out float4 oPosition : POSITION
	,out float2 oUV :TEXCOORD0)
{
	oPosition = mul(wvp, position);
	oUV = uv;
}

void ssaoMultiply_ps(in float4 position : POSITION
	,in float2 uv :TEXCOORD0
	,uniform sampler2D tex1 : TEXUNIT0, uniform sampler2D tex2 : TEXUNIT1
	,out float4 oColor : COLOR)
{
	oColor = tex2D(tex1, uv) * tex2D(tex2, uv);
}
