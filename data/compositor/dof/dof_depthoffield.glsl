#version 120

uniform float far;
uniform vec4 dofparams;       

float convertDepth(float d)
{
	float depth = d*far;
	float depthResult;
	if (depth < dofparams.y)
    {
       // scale depth value between near blur distance and focal distance to
       // [-1, 0] range
       depthResult = (depth - dofparams.y) / (dofparams.y - dofparams.x);
     }
    else
    {
       // scale depth value between focal distance and far blur distance to
       // [0, 1] range
       depthResult = (depth - dofparams.y) / (dofparams.z - dofparams.y);
       // clamp the far blur to a maximum blurriness
       depthResult = clamp(depthResult, 0.0, dofparams.w);
    }
    // scale and bias into [0, 1] range
	depthResult = 0.5f*depthResult + 0.5f;
	return depthResult;
}

uniform sampler2D sceneTex;               // full resolution image
uniform sampler2D depthTex;               // full resolution image with depth values
uniform sampler2D blurTex;                 // downsampled and blurred image

uniform vec4 pixelSize;


varying vec2 UV;

void main(void)
{
vec2 pixelSizeScene = pixelSize.xy;// pixel size of full resolution image
vec2 pixelSizeBlur = pixelSize.zw;// pixel size of downsampled and blurred image

	vec2 poisson[12] = vec2[12](               // containts poisson-distributed positions on the unit circle
		vec2( 0.00,  0.00),
		vec2( 0.07, -0.45),
		vec2(-0.15, -0.33),
		vec2( 0.35, -0.32),
		vec2(-0.39, -0.26),
		vec2( 0.10, -0.23),
		vec2( 0.36, -0.12),
		vec2(-0.31, -0.01),
		vec2(-0.38,  0.22),
		vec2( 0.36,  0.23),
		vec2(-0.13,  0.29),
		vec2( 0.14,  0.41)
	);
  
	vec2 maxCoC;                          // maximum circle of confusion (CoC) radius
							               // and diameter in pixels
	maxCoC = vec2(5.0, 10.0);

	float radiusScale;                      // scale factor for minimum CoC size on low res. image
	radiusScale = 0.4;

	// Get depth of center tap and convert it into blur radius in pixels
	float centerDepth = convertDepth(texture2D(depthTex, UV).r);

	float discRadiusScene = abs(centerDepth * maxCoC.y - maxCoC.x);
	float discRadiusBlur = discRadiusScene * radiusScale; // radius on low res. image

	vec4 sum = vec4(0.0,0.0,0.0,0.0);

	for (int i = 0; i < 12; ++i)
	{
		// compute texture coordinates
		vec2 coordScene = UV + (pixelSizeScene * poisson[i] * discRadiusScene);
		vec2 coordBlur = UV + (pixelSizeBlur * poisson[i] * discRadiusBlur);

		// fetch taps and depth
		vec4 tapScene = texture2D(sceneTex, coordScene);
		float tapDepth = convertDepth(texture2D(depthTex, coordScene).x);
		vec4 tapBlur = texture2D(blurTex, coordBlur);

		// mix low and high res. taps based on tap blurriness
		float blurAmount = abs(tapDepth * 2.0 - 1.0); // put blurriness into [0, 1]
		vec4 tap = mix(tapScene, tapBlur, blurAmount);

		// "smart" blur ignores taps that are closer than the center tap and in focus
		//float factor = (tapDepth >= centerDepth) ? 1.0 : abs(tapDepth * 2.0 - 1.0);
		float factor=1.f;
		// accumulate
		sum.rgb += tap.rgb * factor;
		sum.a += factor;
	}

	gl_FragColor = (sum / sum.a);

}
