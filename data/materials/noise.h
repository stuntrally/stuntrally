//  Array and textureless 2D simplex noise function
//  Author:  Ian McEwan, Ashima Arts
//  License: MIT  https://github.com/ashima/webgl-noise

float3 mod289(float3 x) {	return x - floor(x * (1.0 / 289.0)) * 289.0;  }
float2 mod289(float2 x) {	return x - floor(x * (1.0 / 289.0)) * 289.0;  }

float3 permute(float3 x) {	return mod289( ((x*34.0) + 1.0) * x);  }

float snoise1(float2 v)
{
	const float4 C = float4(
		0.211324865405187,  // (3.0-sqrt(3.0))/6.0
		0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
		-0.577350269189626, // -1.0 + 2.0 * C.x
		0.024390243902439); // 1.0 / 41.0

	//  First corner
	float2 i = floor(v + dot(v, C.yy));
	float2 x0 = v - i + dot(i, C.xx);

	//  Other corners
	float2 i1;
	i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
	// x0 = x0 - 0.0 + 0.0 * C.xx;
	// x1 = x0 - i1  + 1.0 * C.xx;
	// x2 = x0 - 1.0 + 2.0 * C.xx;
	float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	//  Permutations
	i = mod289(i);  // Avoid truncation effects
	float3 p = permute( permute(
		i.y + float3(0.0, i1.y, 1.0)) +
		i.x + float3(0.0, i1.x, 1.0));

	float3 m = max(0.5 - float3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m*m;  m = m*m;

	//  Gradients: 41 points uniformly over a line, mapped onto a diamond.
	//  The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
	float3 x = 2.0 * shFract(p * C.www) - 1.0;
	float3 h = abs(x) - 0.5;
	float3 ox = floor(x + 0.5);
	float3 a0 = x - ox;

	//  Normalise gradients implicitly by scaling m
	//  Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

	//  Compute final noise value at P
	float3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	//return 130.0 * dot(m, g);  // -1..1
	return 0.5 + 0.5 * 130.0 * dot(m, g);  // 0..1
}

float snoise(float2 v, float zoom, int octaves, float persistence)
{
    float total = 0.0;
    for (int i=0; i < 5; ++i)  // const loop
    {
        float frequency = pow(2.f, float(i));
        float amplitude = pow(persistence, float(i));
        float nval = snoise1(v * frequency * zoom) * amplitude;
        total += i < octaves ? nval : 0.f;
    }
    //return total;
    float m = (persistence - 0.1f) * -0.83f + 1.f;
    return total * m;
    // pers = 0.7, mul = 0.5  pers = 0.1, mul = 1
}

//     xa  xb
//1    .___.
//0__./     \.___
//   xa-s    xb+s   // val, min, max, smooth range
float linRange(float x, float xa, float xb, float s)
{
	return shSaturate(x < xa ? (x-xa)/s+1.f : (xb-x)/s+1.f);
}
