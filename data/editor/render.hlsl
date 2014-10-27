//-------------------------------------------------------
struct VIn
{
	float4 p : POSITION;	float3 n : NORMAL;
	float3 uv: TEXCOORD0;
	float4 c : COLOR;
};
struct VOut
{
	float4 p : POSITION;	float3 uv : TEXCOORD0;	float4 wp : TEXCOORD1;
	float3 n : TEXCOORD2;
	float4 c : COLOR;
};
struct PIn
{							float3 uv : TEXCOORD0;	float4 wp : TEXCOORD1;
	float3 n : TEXCOORD2;
	float4 c : COLOR;
};

///-----------------------------------------------------------------------------------------------------------------
///-------------------------------------------------  ppx  vs  render
///-----------------------------------------------------------------------------------------------------------------
VOut render_vs(VIn IN,
	uniform float4x4 wMat,  uniform float4x4 wvpMat)
{
	VOut OUT;  OUT.uv = IN.uv;
	OUT.wp = mul(wMat, IN.p);
	OUT.p = mul(wvpMat, IN.p);
	OUT.n = IN.n;
	OUT.c = IN.c;  //clr
	return OUT;
}

//-------------------------------------------------  ppx  render  -------------------------------------------------
float4 render_ps(PIn IN,      /// _road for minimap_
	uniform float3 ambient,	uniform float4 matDif): COLOR0
{
	float bridge = IN.c.x, pipe = IN.c.y;
	float norm = abs(IN.n.y);  //abs

	float onP = pipe > 0.5f ? 1.f : 0.f;  // get onP from pipe
	pipe = pipe > 0.5f ? min(1.f, (pipe - 0.5f) * 3.5f) : pipe * 2.f;
													//\par
	float pwr = lerp(lerp(8, 8, bridge), 4, pipe);
	float ter = lerp(lerp(1, 0, bridge), 0, pipe);
	float diffuse = 1 - lerp( 1-lerp(pow(norm, pwr), pow(norm, pwr), pipe),
		pow(1.f - 2.f*acos(norm)/3.141592654, pwr), ter);
	float3 clrLi = ambient + diffuse * matDif.rgb;
	
	///  color  for minimap preview
	//  ---~~~====~~~---
	float3 clr =
		lerp(
			lerp(float3(1, 1,   1) * clrLi,           // gray
				 float3(0, 0.8, 1) * (0.4+0.7*clrLi)  // cyan
				 ,bridge),
			lerp(float3(1, 0.8, 0),                         // yellow
				 float3(1, 0.4, 0), onP) * (0.2+1.0*clrLi)  // orange
		,pipe);
		
	return float4(clr, 1);
}

//-------------------------------------------------
float4 render_gr_ps(PIn IN): COLOR0
{
	float bridge = IN.c.w;

	//return IN.c;  // test
	return float4(bridge * float3(1,1,1), bridge);
}
