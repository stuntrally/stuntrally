
void SoftParticlesBlend_vs(in float4 position : POSITION
	,in float2 uv :TEXCOORD0
	,uniform float4x4 wvp
	,out float4 oPosition : POSITION
	,out float2 oUV :TEXCOORD0)
{
	oPosition = mul(wvp, position);
	oUV = uv;
	return;
}

void SoftParticlesBlend_ps(in float4 position : POSITION
	,in float2 uv :TEXCOORD0
	,uniform sampler2D tex1 : TEXUNIT0
	,out float4 oColor : COLOR)
{
	float4 particlecolor = tex2D(tex1, uv);

	oColor =  particlecolor;
	return;
}

void ShowDepth_ps(in float4 position : POSITION
	,in float2 uv :TEXCOORD0
	,uniform sampler2D tex1 : TEXUNIT0
	,uniform float far
	,out float4 oColor : COLOR)
{

	float4 color=tex2D(tex1, uv);
	color=float4(color.x*far/20,color.x*far/20,color.x*far/20,1);
color=sqrt(color);
//	test -only show particles
	oColor =  color;

	return;
}