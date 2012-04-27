void GodRays_VS(
    float4 position			: POSITION,
	float2 uv               : TEXCOORD0,
	out float4 oPos		    : POSITION,
	out float2 oTexCoord	: TEXCOORD0,
	out float2 oLightPos    : TEXCOORD1,
	uniform float4x4 wvp,
  	uniform float4 lightPosition
) {
    position.xy = sign(position.xy);

    oTexCoord = (float2(position.x, -position.y) + 1.0f) * 0.5f;

    oPos = mul(wvp, position);

	
	oLightPos = lightPosition.xy;
}

void GodRays_FP(
     float4 iPos		    : POSITION,
	 float2 uv			: TEXCOORD0,
	float2 lightPosition : TEXCOORD1,
	uniform sampler2D frame : register(s0),
	uniform float enableEffect,
    out float4 color		: COLOR
) {
	const float density = 0.2;
	const int samples = 16;
	const float weight = 0.8;
	const float decay = 1.05;
	
	float2 deltaTexCoord = (uv - lightPosition);
	
	deltaTexCoord *= 1.0f / samples * density;
	
	float3 col = tex2D(frame, uv).rgb;
	float illuminationDecay = 1.0f;
	
	for (int i = 0; i < samples; i++) {
		uv -= deltaTexCoord;
		float3 sample = tex2D(frame, uv).rgb;
		sample *= illuminationDecay * weight;
		col += sample;
		illuminationDecay *= decay;
	}
	
	color = float4(col * 0.04, 1) *enableEffect;
}



void GodRaysOcclude_FP(
     float4 iPos		    : POSITION,
	 float2 uv			: TEXCOORD0,
	float2 lightPosition : TEXCOORD1,
	uniform sampler2D frame : register(s0),
	uniform float enableEffect,
    out float4 color		: COLOR
) {
	const float density = 0.2;
	const int samples = 16;
	const float weight = 0.8;
	const float decay = 1.05;
	
	float2 deltaTexCoord = (uv - lightPosition);
	
	deltaTexCoord *= 1.0f / samples * density;
	
	//the MRT green channel include the luminance info
	float3 raysColor =float3(1.0,1.0,1.0);
	float3 col = tex2D(frame, uv).g *raysColor;
	float illuminationDecay = 1.0f;
	
	for (int i = 0; i < samples; i++) {
		uv -= deltaTexCoord;
		float3 sample = tex2D(frame, uv).g*raysColor;
		sample *= illuminationDecay * weight;
		col += sample;
		illuminationDecay *= decay;
	}
	
	color = float4(col * 0.04, 1) *enableEffect;
}
