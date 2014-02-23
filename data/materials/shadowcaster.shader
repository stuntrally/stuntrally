#include "core.h"

#define ALPHA  @shPropertyBool(shadow_transparency)
#define INSTANCING  @shPropertyBool(instancing)

#ifdef SH_VERTEX_SHADER

    SH_BEGIN_PROGRAM
		#if ALPHA
        shVertexInput(float2, uv0)
        shOutput(float2, UV)
		#endif
	#if INSTANCING
		shVertexInput(float4, uv1)
		shVertexInput(float4, uv2)
		shVertexInput(float4, uv3)
        shUniform(float4x4, vp)   @shAutoConstant(vp, viewproj_matrix)
 	#else
        shUniform(float4x4, wvp)  @shAutoConstant(wvp, worldviewproj_matrix)
	#endif
        shOutput(float2, depth)

    SH_START_PROGRAM
    {
		#if INSTANCING
			float4x4 world;
			world[0] = uv1;
			world[1] = uv2;
			world[2] = uv3;
			world[3] = float4(0,0,0,1);
			float4 wpos = float4( shMatrixMult(world, shInputPosition).xyz, 1.0f);
			//  transform the position
			shOutputPosition = shMatrixMult( vp, wpos );
		#else
			//  this is the view space position
			shOutputPosition = shMatrixMult(wvp, shInputPosition);
	    #endif

	    //  depth info for the fragment
	    depth.x = shOutputPosition.z;
	    depth.y = shOutputPosition.w;

	    //  clamp z to zero. seem to do the trick
	    shOutputPosition.z = max(shOutputPosition.z, 0);

		#if ALPHA
	    UV = uv0;
		#endif
    }

#else

    SH_BEGIN_PROGRAM
		#if ALPHA
        shInput(float2, UV)
        shSampler2D(texture1)
		#endif
        shInput(float2, depth)

    SH_START_PROGRAM
    {
	    float d = depth.x / depth.y;  // finalDepth

		#if ALPHA
        //  use alpha channel of the first texture
        float alpha = shSample(texture1, UV).a;

        if (alpha < 0.5)
            discard;
		#endif

	    shOutputColour(0) = float4(d,d,d,1);
    }

#endif
