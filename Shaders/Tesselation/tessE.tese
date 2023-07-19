// tessellation evaluation shader
#version 450


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    
} ubo;
layout(location = 1) in vec4[] inColor ;
layout(location = 1) out vec4 outColor ;
layout (quads) in;
layout(set = 1, binding = 0) uniform sampler2D u_Tex;
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
float norm(float val, float max,float min)
{
     return (val- min) / (max - min);
}

float noise ( vec2 cell, vec2 uv, float nextVertexOffset) {
    vec2 i = cell;
    vec2 f = uv;
   // i = vec2(gl_InstanceIndex,0);
    float a = random(i);
   float b = random(i + vec2(2.5, 0.0));
    float c = random(i + vec2(0.0, 2.5));
    float d = random(i + vec2(2.5,2.5));

	// smooth step function lol
    vec2 u = f * f * (3.0 - 2.0 * f);
    //u = vec2(1,1);
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
#define OCTAVES 37
float fbm(vec2 cell,vec2 newuv, float nextVertexOffset)
{
	// Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 1;

	  // Loop of octaves
    for (int i = 0; i < OCTAVES; i++)
	{
        value += amplitude * noise(cell  *frequency,newuv, nextVertexOffset);
        //st *=3.;
        amplitude *= .3;
    }
    return value;
}
void main()
{
   // Calculate the barycentric coordinates of the current point on the quad
    vec3 barycentricCoord = gl_TessCoord.xyz;
    // Interpolate the position within the quad using barycentric coordinates
    vec4 position = mix(
        mix(gl_in[0].gl_Position, gl_in[1].gl_Position, barycentricCoord.x),
        mix(gl_in[2].gl_Position, gl_in[3].gl_Position, barycentricCoord.x),
        barycentricCoord.z
    );
    // Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.z);


// get patch coordinate
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
// retrieve control point position coordinates
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

// bilinearly interpolate position coordinate across patch
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;

    float gridSize = 100;
    float tileWidth = length(gl_in[0].gl_Position - gl_in[1].gl_Position);
   
    vec2 newUv = GetUvs(p.xz,gridSize, vec2(u,v),gridSize / (tileWidth));

    float noiseValue = texture(u_Tex,newUv).x;
   
     vec4 color = mix(
        mix(inColor[0], inColor[1], barycentricCoord.x),
        mix(inColor[3], inColor[2], barycentricCoord.x),
        barycentricCoord.z
    );
     p.y =abs(noiseValue);

     //p.y =color.x;
     outColor = vec4(newUv,0,1);
     outColor =vec4(noiseValue,noiseValue,noiseValue,1);
    gl_Position = ubo.proj*ubo.view*p;
}