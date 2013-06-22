#include "core.h"

#ifdef SH_VERTEX_SHADER

	attribute vec4 vertex;
	uniform mat4  wvp; @shAutoConstant(wvp, worldviewproj_matrix)

	attribute vec2 uv0;
	varying vec2 UV;

	void main(void)
	{
		gl_Position = (wvp * vertex);
		UV = uv0;
	}

#else

	varying vec2 UV;
	uniform sampler2D scene; @shUseSampler(scene)
	uniform vec4 inverseTextureSize; @shAutoConstant(inverseTextureSize, inverse_texture_size, 0)

	void main(void)
	{
		vec4 colour = vec4(0,0,0,0);
		float blurSize = inverseTextureSize.y;

		//X-blur.
		colour += texture2D(scene, vec2(UV.x, UV.y - 4.0*blurSize)) * 1.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y - 3.0*blurSize)) * 2.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y - 2.0*blurSize)) * 3.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y - blurSize)) * 4.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y)) * 5.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y + blurSize)) * 4.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y + 2.0*blurSize)) * 3.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y + 3.0*blurSize)) * 2.0/25.0;
		colour += texture2D(scene, vec2(UV.x, UV.y + 4.0*blurSize)) * 1.0/25.0;
	 
		gl_FragColor = colour;
	}

#endif
