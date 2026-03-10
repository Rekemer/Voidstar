#version 450

layout(location = 0) out vec4 color ;
layout(location = 0) in vec2 out_uv;
layout(location = 1) in vec3 out_worldPos;


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;



layout(set = 1, binding = 0) uniform sampler2D u_Texture;
layout(set = 1, binding = 1) uniform sampler2D u_Grid;
layout(set = 1, binding = 2) uniform sampler2D u_Noise;
//layout(set = 1, binding = 1) uniform sampler2D u_Texture1;
//layout(set = 2, binding = 0) uniform sampler2D u_Texture1;

float sampleFieldBilinear(vec2 uv)
{
    vec2 texSize = textureSize(u_Grid,0).xy;
    // uv in [0,1]
    vec2 texelPos = uv * texSize - vec2(0.5);
    vec2 base = floor(texelPos);
    vec2 f = fract(texelPos);

    vec2 uv00 = (base + vec2(0.5, 0.5)) / texSize;
    vec2 uv10 = (base + vec2(1.5, 0.5)) / texSize;
    vec2 uv01 = (base + vec2(0.5, 1.5)) / texSize;
    vec2 uv11 = (base + vec2(1.5, 1.5)) / texSize;

    float a = texture(u_Grid, uv00).r;
    float b = texture(u_Grid, uv10).r;
    float c = texture(u_Grid, uv01).r;
    float d = texture(u_Grid, uv11).r;

    float x0 = mix(a, b, f.x);
    float x1 = mix(c, d, f.x);
    return mix(x0, x1, f.y);
}

void main() 
{
    vec2 noiseVec = texture(u_Noise, fract(out_uv * 5.0)).rg * 2.0 - 1.0;
    vec2 distortedUV = out_uv + noiseVec * 0.008;

    float field = sampleFieldBilinear(distortedUV);
    float baseCoverage = smoothstep(0.2, 0.6, field);

    float noiseVal = texture(u_Noise, fract(out_uv * 3.0)).r;
    float edgeBand = 1.0 - abs(baseCoverage * 2.0 - 1.0);

    float shapedField = field + (noiseVal - 0.5) * 0.06 * edgeBand;
    float coverage = smoothstep(0.2, 0.4, shapedField);

    color = vec4(vec3(coverage), 1.0);

}