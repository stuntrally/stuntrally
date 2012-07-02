#if SH_HLSL == 1 || SH_CG == 1

	#define shTexture2D sampler2D
	#define shSample(tex, coord) tex2D(tex, coord)
	#define shLerp(a, b, t) lerp(a, b, t)
	#define shSaturate(a) saturate(a)

	#define shMatrixMult(m, v) mul(m, v)

	#define shUniform(s) , uniform s


	#ifdef SH_VERTEX_SHADER

		#define shOutputPosition oPosition
		#define shInputPosition iPosition


		#define SH_BEGIN_PROGRAM \
			void main( \
				  float4 iPosition : POSITION \
				, out float4 oPosition : POSITION

		#define SH_START_PROGRAM \
			) \
		
	#endif

	#ifdef SH_FRAGMENT_SHADER

		#define shOutputColor oColor

		#define SH_BEGIN_PROGRAM \
			void main( \
				out float4 oColor : COLOR

		#define SH_START_PROGRAM \
			) \

	#endif

#endif

#if SH_GLSL == 1
	@shGlslVersion(130)

	#define float2 vec2
	#define float3 vec3
	#define float4 vec4
	#define int2 ivec2
	#define int3 ivec3
	#define int4 ivec4
	#define shTexture2D sampler2D
	#define shSample(tex, coord) texture(tex, coord)
	#define shLerp(a, b, t) mix(a, b, t)
	#define shSaturate(a) clamp(a, 0.0, 1.0)

	#define shUniform(s) uniform s;

	#define shMatrixMult(m, v) m * v

	#define shInputPosition vertex
	#define shOutputPosition gl_Position
	#define shOutputColor ocolour

	#define float4x4 mat4

	#ifdef SH_VERTEX_SHADER

		#define SH_BEGIN_PROGRAM \
			in float4 vertex;
		#define SH_START_PROGRAM \
			void main(void)
		
	#endif

	#ifdef SH_FRAGMENT_SHADER

		#define SH_BEGIN_PROGRAM \
			out float4 ocolour;
		#define SH_START_PROGRAM \
			void main(void)

		#define SH_INPUT(name, type) in type name // produces something like "in vec3 color"
		#define SH_INPUT(name, type) in type name // produces something like "in vec3 color"

	#endif
#endif
