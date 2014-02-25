void FilmGrain_VS(
    float4 position			: POSITION,
	float2 uv               : TEXCOORD0,
	out float4 oPos		    : POSITION,
	out float2 oTexCoord	: TEXCOORD0
) {
    oPos = float4(position.xy, 0, 1);
	oTexCoord = uv;
	
}

float3 Overlay(float3 a, float3 b){
    return pow(abs(b), 2.2) < 0.5? 2 * a * b : 1.0 - 2 * (1.0 - a) * (1.0 - b);
}

void FilmGrain_FP(
     float4 iPos		    : POSITION,
	 float2 uv			: TEXCOORD0,
	uniform sampler2D frame : TEXUNIT0,
	uniform sampler3D noiseTex : TEXUNIT1,
    uniform float4 grainparams,
    uniform float time,
   out float4 color		: COLOR

)
{	

	float2 pixelSize = grainparams.xy;
	float noiseIntensity = grainparams.w;//grainparams.z;
	float exposure = 1;//grainparams.w;
	float3 framecolor= tex2D (frame,uv).rgb;
	float2 coord = uv * 2.0;
    coord.x *= pixelSize.y / pixelSize.x;
    float noise = tex3D(noiseTex, float3(coord, time)).r;
    float exposureFactor = exposure / 2.0;
    exposureFactor = sqrt(exposureFactor);
    float t = lerp(3.5 * noiseIntensity, 1.13 * noiseIntensity, exposureFactor);
    color = float4(Overlay(framecolor, lerp(0.5, noise, t)),1.0);
	
}


