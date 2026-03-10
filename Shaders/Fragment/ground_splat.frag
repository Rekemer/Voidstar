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
    vec2 texSize = vec2(1024/2);
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

  
    vec4 tex = texture(u_Texture,vec2(fract(out_uv * 5)));
    vec4 grid = texture(u_Grid,vec2(out_uv));
    vec4 noise = texture(u_Noise,vec2(fract(out_uv * 5)));

    vec2 distortion = vec2(noise.x - 0.5) * 0.05; 
    vec2 distortedUV = out_uv + distortion;

    float field = sampleFieldBilinear(out_uv);
 

    // shape it into a cleaner blob
    //float coverage = smoothstep(0.2, 0.6, noise.x*field);
    float coverage = step(0.3, noise.x*field);
    vec3 surfaceColor = vec3(1.0, 1.0, 1.0);
    vec3 finalColor = mix(vec3(0,0,0),surfaceColor, coverage);
   //if (out_uv.x < 0.5)
   //color = vec4(vec3(field), 1.0);
   //else
    color = vec4(finalColor, 1.0);
    //color.xyz = mix(vec3(0,0,0),surfaceColor, noise.xyz);
    //color.xyz = coverage * noise.xyz; 
    color.a = 1;

}