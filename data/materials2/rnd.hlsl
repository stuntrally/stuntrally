
struct iVIn
{
	float4 p	: POSITION;
	float2 uv   : TEXCOORD0;
};
struct iVOut
{
	float4 p	: POSITION;
	float2 uv	: TEXCOORD0;
};


iVOut rnd_vs(iVIn IN,
	uniform float4x4 wvpMat)
{
	iVOut OUT;  OUT.uv = IN.uv;
	OUT.p = mul(wvpMat, IN.p);
	return OUT;
}


float4 rnd_ps(in float4 p	: POSITION,in float2 uv : TEXCOORD0,
	uniform sampler2D diffuseMap : TEXUNIT0): COLOR0
{
	return tex2D(diffuseMap, uv);
}


//  rot, inv
float4 rnd_inv_ps(in float4 p	: POSITION,in float2 uv : TEXCOORD0,
	uniform sampler2D diffuseMap : TEXUNIT0): COLOR0
{
	float2 uv2 = float2(uv.y, 1.f - uv.x);  // rotate 90'
	return float4(1,1,1,0) -/**/ tex2D(diffuseMap, uv2);  // inverse
}


//  add 2 tex
float4 rnd_add_ps(in float4 p	: POSITION,in float2 uv : TEXCOORD0,
	uniform sampler2D diffuseAdd : TEXUNIT0,
	uniform sampler2D diffuseMap : TEXUNIT1): COLOR0
{
	float4 rd = tex2D(diffuseAdd, uv);
	float4 ter = tex2D(diffuseMap, uv);
	return float4(lerp(rd.rgb, ter.rgb, 1-rd.a), 1);
}


//  zoomed minimap circle  ----------------------

struct CIn
{
	float4 p	: POSITION;
	float2 uv   : TEXCOORD0;
	float4 clr  : COLOR;
};
struct COut
{
	float4 p	: POSITION;
	float2 uv	: TEXCOORD0;
	float4 clr  : COLOR;
};

COut circle_vs(CIn IN,
	uniform float4x4 wvpMat)
{
	COut OUT;  OUT.uv = IN.uv;  OUT.clr = IN.clr;
	OUT.p = mul(wvpMat, IN.p);
	return OUT;
}

float4 circle_ps(in float4 p : POSITION,
				in float2 uv : TEXCOORD0,
				in float4 clr : COLOR,  // tc 0..1
	uniform float showTerrain,  // bool
	uniform float showBorder,
	uniform float square,
	uniform sampler2D roadMap : TEXUNIT0,
	uniform sampler2D circleMap : TEXUNIT1,
	uniform sampler2D terMap : TEXUNIT2): COLOR0
{
	float4 road = tex2D(roadMap, uv);
	float4 cir = square > 0.5f ? float4(1,0,1,1) : tex2D(circleMap, clr.xy);
	float4 ter = tex2D(terMap, uv);

	float3 cl, bcl = float3(1.1,1.4,1.5);  //border color
	float4 c;  float a,b;

	if (showBorder > 0.5f)
	{
		if (showTerrain > 0.5f)
		{
			a = cir.a;
			cl = lerp(ter.rgb,road.rgb, road.a);
			c = float4(lerp(cir.g * bcl, cl.rgb, cir.r), a);
		}else
		{
			a = road.a * cir.a;  b = cir.a - cir.r*0.8f;  //par alpha clear
			cl = road.rgb;
			c = float4(lerp(cir.g * bcl, cl, cir.r), max(a, b));
		}
	}else
	{
		if (showTerrain > 0.5f)
		{
			a = cir.b;
			c = float4(lerp(ter.rgb,road.rgb, road.a), a);
		}else
		{
			a = cir.b;
			c = float4(road.rgb, road.a * a);
		}
	}
	return c;
}
