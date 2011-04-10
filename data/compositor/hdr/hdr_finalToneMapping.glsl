uniform sampler2D inRTT;
uniform sampler2D inBloom;
uniform sampler2D inLum;

varying vec2 uv;

// declare external function
vec4 toneMap(in vec4 inColour, in float lum);

void main(void)
{
	// Get main scene colour
    vec4 sceneCol = texture2D(inRTT, uv);

	// Get luminence value
	vec4 lum = texture2D(inLum, vec2(0.5, 0.5));

	// tone map this
	vec4 toneMappedSceneCol = toneMap(sceneCol, lum.r);
	
	// Get bloom colour
    vec4 bloom = texture2D(inBloom, uv);

	// Add scene & bloom
	///  params...
	gl_FragColor = vec4(toneMappedSceneCol.rgb*1.0 + toneMappedSceneCol.rgb*bloom.rgb*0.8 + 0.3*bloom.rgb, 1.0);
	//gl_FragColor = vec4(toneMappedSceneCol.rgb + bloom.rgb, 1.0);
    
}

