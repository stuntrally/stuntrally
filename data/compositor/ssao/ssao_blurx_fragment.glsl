#version 120

varying vec2 uv;
uniform vec4 invTexSize;
uniform sampler2D map;
uniform sampler2D geomMap;


#define NUM_BLUR_SAMPLES 8

void main(void)
{
    vec2 o = vec2(invTexSize.x, 0);
    vec4 sum = texture2D(map, uv) * (NUM_BLUR_SAMPLES + 1);
    float denom = NUM_BLUR_SAMPLES + 1;
    vec4 geom = texture2D(geomMap, uv);
    for (int i = 1; i <= NUM_BLUR_SAMPLES; ++i)
    {
        vec2 nuv = uv + o * i;
        vec4 nGeom = texture2D(geomMap, nuv);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * (dot(geom.yzw, nGeom.yzw) > 0.9 ? 1.0 : 0.0);
        sum += texture2D(map, nuv) * coef;
        denom += coef;
    }
    for (int i = 1; i <= 4; ++i)
    {
        vec2 nuv = uv + o * -i;
        vec4 nGeom = texture2D(geomMap, nuv);
        float coef = (NUM_BLUR_SAMPLES + 1 - i) * (dot(geom.yzw, nGeom.yzw) > 0.9 ? 1.0 : 0.0);
        sum += texture2D(map, nuv) * coef;
        denom += coef;
    }
    gl_FragColor = sum / denom;
}
