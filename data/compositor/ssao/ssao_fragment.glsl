#version 120

varying vec2 uv;
varying vec3 ray;

uniform sampler2D geomMap;
uniform sampler2D randMap;

uniform float far;
uniform vec4 fogParams;
uniform mat4 ptMat;

void main(void)
{
    #define MAX_RAND_SAMPLES 14

    const vec3 RAND_SAMPLES[MAX_RAND_SAMPLES] = vec3[MAX_RAND_SAMPLES](
        vec3(1, 0, 0),
        vec3(	-1, 0, 0),
        vec3(0, 1, 0),
        vec3(0, -1, 0),
        vec3(0, 0, 1),
        vec3(0, 0, -1),
        normalize(vec3(1, 1, 1)),
        normalize(vec3(-1, 1, 1)),
        normalize(vec3(1, -1, 1)),
        normalize(vec3(1, 1, -1)),
        normalize(vec3(-1, -1, 1)),
        normalize(vec3(-1, 1, -1)),
        normalize(vec3(1, -1, -1)),
        normalize(vec3(-1, -1, -1))
    );

    // constant expressin != const int :(
    #define NUM_BASE_SAMPLES 6

    // random normal lookup from a texture and expand to [-1..1]
    vec3 randN = texture2D(randMap, uv * 24).xyz * 2.0 - 1.0;
    vec4 geom = texture2D(geomMap, uv);
    float depth = geom.x;

	float fogAmount = clamp(fogParams.x * (depth*far - fogParams.y) * fogParams.w, 0, 1);

    // IN.ray will be distorted slightly due to interpolation
    // it should be normalized here
    vec3 viewPos = ray * depth;

    // by computing Z manually, we lose some accuracy under extreme angles
    // considering this is just for bias, this loss is acceptable
    vec3 viewNorm = geom.yzw;//computeZ(geom.yz);


        mat4 clipSpaceMatrix = mat4(0.5, 0.0, 0.0, 0.0, 
                         0.0, -0.5, 0.0, 0.0,
                         0.0, 0.0, 0.5, 0.0,
                         0.5, 0.5, 0.0, 1.0);

    // accumulated occlusion factor
    float occ = 0;
    for (int i = 0; i < NUM_BASE_SAMPLES; ++i)
    {
        // reflected direction to move in for the sphere
        // (based on random samples and a random texture sample)
        // bias the random direction away from the normal
        // this tends to minimize self occlusion
        vec3 randomDir = reflect(RAND_SAMPLES[i], randN) + viewNorm;

        // move new view-space position back into texture space
        #define RADIUS 0.2125
        vec4 nuv = (ptMat * vec4(viewPos.xyz + randomDir * RADIUS, 1));
        nuv.xy /= nuv.w;

        // compute occlusion based on the (scaled) Z difference
        float zd = clamp(far * (depth - texture2D(geomMap, nuv.xy).x), 0, 1);
        // this is a sample occlusion function, you can always play with
        // other ones, like 1.0 / (1.0 + zd * zd) and stuff
        occ += clamp(pow(1.0 - zd, 11) + zd, 0, 1);
    }
    occ /= NUM_BASE_SAMPLES;

    gl_FragColor = vec4( mix( vec3(occ, occ, occ), vec3(1,1,1), fogAmount), 1);
}
