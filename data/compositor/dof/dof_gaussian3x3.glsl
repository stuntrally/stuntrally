#version 120

uniform sampler2D sourceTex;
uniform vec4 pixelSize;

varying vec2 UV;

void main(void)
{
	const float weights[9] = float[9](
		1.0/16.0, 2.0/16.0, 1.0/16.0,
		2.0/16.0, 4.0/16.0, 2.0/16.0,
		1.0/16.0, 2.0/16.0, 1.0/16.0
	);
	vec2 offsets[9] = vec2[9](
		vec2(-pixelSize.z, -pixelSize.w),
		vec2(0, -pixelSize.w),
		vec2(pixelSize.z, -pixelSize.w),
		vec2(-pixelSize.z, 0),
		vec2(0, 0),
		vec2(pixelSize.z, 0),
		vec2(-pixelSize.z, pixelSize.w),
		vec2(0,  pixelSize.w),
		vec2(pixelSize.z, pixelSize.w)
	);

	vec4 sum = vec4(0,0,0,0);

	for (int i = 0; i < 9; ++i)
		sum += weights[i] * texture2D(sourceTex, UV + offsets[i]);

	gl_FragColor = sum;
}
