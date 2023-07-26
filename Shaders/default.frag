#version 450

layout(location = 0) out vec4  outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv ;
//layout(location = 2) in float scale;
//layout(location = 3) in vec4 worldSpacePos;
//layout(location = 4) in float depth;


layout(set = 1, binding = 0) uniform sampler2D u_Noise;
layout(set = 1, binding = 1) uniform sampler2D u_Tex1;
layout(set = 1, binding = 2) uniform sampler2D u_Tex2;
layout(set = 1, binding = 3) uniform sampler2D u_Tex3;

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
    vec4 snowTex = texture(u_Tex1,uv);
    vec4 groundTex = texture(u_Tex2,uv);
    vec4 stoneTex = texture(u_Tex3,uv);
    float currentVertexHeight  = color.x;
    float maxVertexHeight  = color.y;
    float level = mix(0,maxVertexHeight,currentVertexHeight);
    float noise = level;
    outColor.xyz = vec3(noise ,noise ,noise );




//     float weight0 = smoothstep(0.0, 1.0, level);  // Example: Blend texture 0 based on height
//     float weight1 = 1.0 - weight0;                // Inverse of weight0

//    // float weight2 = 1.0 - weight0 - weight1;       // Blend texture 2 with remaining weight

//     // Sample each texture and blend them based on the weights
    
//     // Calculate the final color by blending the textures based on the weights
//     //vec4 finalColor = groundTex * weight1 + stoneTex * weight0;

//     // Output the final color
//  // Define height thresholds for blending
//     float grassThreshold = 0.0;    // Height at which grass starts
//     float stoneThreshold = .5*maxVertexHeight;   // Height at which stone takes over
//     float snowThreshold = .7*maxVertexHeight;    // Height at which snow starts

//     // Define colors for grass, snow, and stone
//     vec3 grassColor = vec3(0.2, 0.7, 0.2);     // Green color for grass
//     vec3 snowColor = vec3(1.0, 1.0, 1.0);      // White color for snow
//     vec3 stoneColor = vec3(0.5, 0.5, 0.5);     // Gray color for stone

//     // Calculate blending weights based on the height
//     float grassWeight = smoothstep(0, grassThreshold, currentVertexHeight);
//     float stoneWeight = smoothstep(grassThreshold, stoneThreshold, currentVertexHeight);
//     float snowWeight = smoothstep(stoneThreshold, snowThreshold, currentVertexHeight);

//     // Calculate the final color by blending the colors based on the weights
//     stoneColor = stoneColor * stoneWeight;
//     vec3 finalColor = grassColor * grassWeight + snowColor * snowWeight +stoneColor;

//     // Output the final color
//     outColor.xyz = finalColor;
    
   
}