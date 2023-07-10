#version 450

layout(location = 0) out vec4  outColor;

//layout(location = 0) in vec2 uv ;
layout(location = 1) in vec4 color;
//layout(location = 2) in float scale;
//layout(location = 3) in vec4 worldSpacePos;
//layout(location = 4) in float depth;

layout(set = 1, binding = 0) uniform sampler2D u_Tex;


vec4 random_color(vec4 st)
{
	float dotProduct1 = dot(st, vec4(127.1, 311.7, 23423.1, 98.2));
    float dotProduct2 = dot(st, vec4(269.5, 183.3, 21.2, 65.1));
    float dotProduct3 = dot(st, vec4(129.5, 383.3, 11.2, 5.1));
    float dotProduct4 = dot(st, vec4(26.5, 83.3, 2.2, 15.1));

    vec4 sinResult = sin(vec4(dotProduct1, dotProduct2,dotProduct3,dotProduct4));
    vec4 multiplied = sinResult * 43758.5453;

    return fract(multiplied);
}


float norm(float val, float max,float min)
 {
     return (val- min) / (max - min);
}

vec4 random(vec4 st)
{
	float dotProduct1 = dot(st, vec4(127.1, 311.7, 23423.1, 98.2));
    float dotProduct2 = dot(st, vec4(269.5, 183.3, 21.2, 65.1));
    float dotProduct3 = dot(st, vec4(129.5, 383.3, 11.2, 5.1));
    float dotProduct4 = dot(st, vec4(26.5, 83.3, 2.2, 15.1));

    vec4 sinResult = sin(vec4(dotProduct1, dotProduct2,dotProduct3,dotProduct4));
    vec4 multiplied = sinResult * 43758.5453;

    return fract(multiplied);
}
float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float noise (vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1., 0.0));
    float c = random(i + vec2(0.0, 1.));
    float d = random(i + vec2(1.,1.));

	// smooth step function lol
    vec2 u = f * f * (3.0 - 2.0 * f);

	//return u.x;
float interpolated;
#define MYVERSION 1
	#ifdef MYVERSION 
	float interpolatedX = mix(a, b, u.x);
	float interpolatedY = mix(c, d, u.x);
	interpolated = mix(interpolatedX , interpolatedY , u.y);
	#else
 	interpolated = mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
	#endif

    return interpolated;
}

vec2 GetUvs(vec2 worldSpacePos, float dimension, vec2 uv, float depth)
{
    
    float scaling = depth/dimension;
    vec2 uv_ = uv * scaling;

    vec2 wp = worldSpacePos.xy;
    float xn = norm(wp.x,dimension/2,-dimension/2);
    float yn = norm(wp.y,dimension/2,-dimension/2);
    
    vec2 newUv = vec2(xn,yn);
	float n = scaling/2;
    return newUv;
}

void main() 
{

  
   // vec2 newUv = GetUvs(worldSpacePos.xz,10,uv,depth);
    //newUv.x  = uv_.x + (xn-n);
    //newUv.y = uv_.y + (yn-n);

    //outColor = random_color(vec4(1,1,1,1));
	outColor.a = 1;
    //outColor.xyz=color.xyz;
    outColor.xyz=vec3(1,1,1);
    //vec2 grid = fract(newUv* 3.);
    outColor.xyz=color.xyz;
    //outColor.xyz=vec3(0);
    //outColor.z =0;
    //outColor.xy = newUv;
    //outColor.xy =fract(newUv*10); 
   // outColor.xyz = vec3(noiseValue,noiseValue,noiseValue  );
    //outColor.xyz= vec3(newUv,0.);
    //outColor.xyz = worldSpacePos.xyz;
    //outColor = vec4(grid,0,1);
    //outColor = texture(u_Tex, uv);
}