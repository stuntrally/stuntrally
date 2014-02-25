
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
};

struct PIn
{
	float4 p   : POSITION; 
    float2 uv  : TEXCOORD0;
};

VOut dof_vs(VIn IN, uniform float4x4 wvp, uniform float3 farCorner)
{
    VOut OUT;
    OUT.p = mul(wvp, IN.p);
    // clean up inaccuracies for the UV coords
    float2 uv = sign(IN.p);
    // convert to image space
    uv = (float2(uv.x, -uv.y) + 1.0) * 0.5;
    OUT.uv = uv;
     return OUT;
}

float4 Gaussian3x3_ps(
PIn IN
,uniform sampler2D sourceTex: TEXUNIT0
,uniform float4 pixelSize
) : COLOR0
{

  float weights[9];
  float2 offsets[9];
  weights[0] = 1.0/16.0; weights[1] = 2.0/16.0; weights[2] = 1.0/16.0;
  weights[3] = 2.0/16.0; weights[4] = 4.0/16.0; weights[5] = 2.0/16.0;
  weights[6] = 1.0/16.0; weights[7] = 2.0/16.0; weights[8] = 1.0/16.0;

  offsets[0] = float2(-pixelSize.x, -pixelSize.y);
  offsets[1] = float2(0, -pixelSize.y);
  offsets[2] = float2(pixelSize.x, -pixelSize.y);
  offsets[3] = float2(-pixelSize.x, 0);
  offsets[4] = float2(0, 0);
  offsets[5] = float2(pixelSize.x, 0);
  offsets[6] = float2(-pixelSize.x, pixelSize.y);
  offsets[7] = float2(0,  pixelSize.y);
  offsets[8] = float2(pixelSize.x, pixelSize.y);

  float4 sum = {0.0,0.0,0.0,0.0};

  for (int i = 0; i < 9; ++i)
    sum += weights[i] * tex2D(sourceTex, IN.uv + offsets[i]);

  return sum;
}

float ConvertDepth(float d, float far,float4 dofParams)
{
//	float4 dofParams = float4(.001,100,1000,1.0);
	//float3 viewPos = d ;
	float depth = d*far;//viewPos.z;
	float depthResult;
	if (depth < dofParams.y)
    {
       // scale depth value between near blur distance and focal distance to
       // [-1, 0] range
     //  depthResult = (depth - dofParams.y) / (dofParams.y - dofParams.x);
	 depthResult=0;
     }
    else
    {
       // scale depth value between focal distance and far blur distance to
       // [0, 1] range
       depthResult = (depth - dofParams.y) / (dofParams.z - dofParams.y);
       // clamp the far blur to a maximum blurriness
       depthResult = clamp(depthResult, 0.0, dofParams.w);
	

    }
    // scale and bias into [0, 1] range
	depthResult = 0.5f*depthResult + 0.5f;
	return depthResult;
}
float4 DepthOfField_ps(
PIn IN
,uniform sampler2D sceneTex: TEXUNIT0                // full resolution image
,uniform sampler2D depthTex: TEXUNIT1                // full resolution image with depth values
,uniform sampler2D blurTex: TEXUNIT2                 // downsampled and blurred image
,uniform float4 pixelSize
,uniform float far
,uniform float4 dofparams          
) : COLOR0
{
	float2 poisson[12];               // containts poisson-distributed positions on the unit circle
float2 pixelSizeScene = pixelSize.xy;// pixel size of full resolution image
float2 pixelSizeBlur = pixelSize.zw;// pixel size of downsampled and blurred image
  poisson[ 0] = float2( 0.00,  0.00);
  poisson[ 1] = float2( 0.07, -0.45);
  poisson[ 2] = float2(-0.15, -0.33);
  poisson[ 3] = float2( 0.35, -0.32);
  poisson[ 4] = float2(-0.39, -0.26);
  poisson[ 5] = float2( 0.10, -0.23);
  poisson[ 6] = float2( 0.36, -0.12);
  poisson[ 7] = float2(-0.31, -0.01);
  poisson[ 8] = float2(-0.38,  0.22);
  poisson[ 9] = float2( 0.36,  0.23);
  poisson[10] = float2(-0.13,  0.29);
  poisson[11] = float2( 0.14,  0.41);
  
  float2 maxCoC;                          // maximum circle of confusion (CoC) radius
                                        // and diameter in pixels
  maxCoC = float2(5.0, 10.0);
  
  float radiusScale;                      // scale factor for minimum CoC size on low res. image
  radiusScale = 0.4;

  // Get depth of center tap and convert it into blur radius in pixels
  float centerDepth = ConvertDepth(tex2D(depthTex, IN.uv).r, far,dofparams);
 // return float4(centerDepth,centerDepth,centerDepth,1);
  float discRadiusScene = abs(centerDepth * maxCoC.y - maxCoC.x);
  float discRadiusBlur = discRadiusScene * radiusScale; // radius on low res. image

  float4 sum = float4(0.0,0.0,0.0,0.0);

  for (int i = 0; i < 12; ++i)
  {
    // compute texture coordinates
    float2 coordScene = IN.uv + (pixelSizeScene * poisson[i] * discRadiusScene);
    float2 coordBlur = IN.uv + (pixelSizeBlur * poisson[i] * discRadiusBlur);
  
    // fetch taps and depth
    float4 tapScene = tex2D(sceneTex, coordScene);
    float tapDepth = ConvertDepth(tex2D(depthTex, coordScene).r ,far,dofparams);
    float4 tapBlur = tex2D(blurTex, coordBlur);
    
    // mix low and high res. taps based on tap blurriness
    float blurAmount = abs(tapDepth * 2.0 - 1.0); // put blurriness into [0, 1]
    float4 tap = lerp(tapScene, tapBlur, blurAmount);
  
    // "smart" blur ignores taps that are closer than the center tap and in focus
    float factor = (tapDepth >= centerDepth) ? 1.0 : abs(tapDepth * 2.0 - 1.0);
  
    // accumulate
    sum.rgb += tap.rgb * factor;
    sum.a += factor;
  }

  return (sum / sum.a);
}
