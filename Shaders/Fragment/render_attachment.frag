#version 450
layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;


layout(set = 1, binding = 0) uniform sampler2D u_Scene;
layout(set = 1, binding = 1) uniform sampler2D u_Noise;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

float catRom(float x, float a,float a1,float a2,float a3)
{
    return pow(x,3) * a3 +pow(x,2) * a2 +pow(x,1) * a1 +a;
}

float scale = 10;
float bicubic(vec2 scaledUv)
{
    vec2 indexUv = floor(scaledUv);
    vec2 f = fract(scaledUv);
   // f = vec2(1)-f;
    float splineX[4][4];

    for (int i =0; i< 4; i++)
    {
        for (int j =0; j< 4; j++)
        {
            
            splineX[i][j] = texture(u_Scene,uv + vec2(i-1,j-1)/(scale/6)).x;
        }
    }
    
    float splineY[4];
    for (int i=0; i <4; i++)
    {
        float ya = splineX[0][i];
        float yb = splineX[1][i];
        float yc = splineX[2][i];
        float yd = splineX[3][i];
        float a0 = yb;
        float a1 = 0.5 *(-ya+yc);
        float a2 = 0.5 * (2*ya-5*yb+4*yc -yd);   
        float a3 = 0.5 * (-ya+3*yb-3*yc +yd);   
        splineY[i] = catRom(f.x,a0,a1,a2,a3);
    }

    float ya = splineY[0];
    float yb = splineY[1];
    float yc = splineY[2];
    float yd = splineY[3];
    float a0 = yb;
    float a1 = 0.5 *(-ya+yc);
    float a2 = 0.5 * (2*ya-5*yb+4*yc -yd);   
    float a3 = 0.5 * (-ya+3*yb-3*yc +yd);   
    float res = catRom(f.y,a0,a1,a2,a3);
    return clamp(res,0,1);

}

void main() {
    
    
   float mask = texture(u_Scene, uv).r;
float noise = texture(u_Noise, uv * 4.0).r;

float edge = smoothstep(0.3, 0.6, mask + noise * 0.15);

vec3 base = texture(u_Scene, uv * 8.0).rgb;
vec3 poisonColor = vec3(0.2, 0.8, 0.2);

vec3 finalColor = mix(vec3(0), poisonColor, edge);
outColor.a = 1;
outColor.xyz = finalColor;
}