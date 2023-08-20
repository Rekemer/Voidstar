#version 450

layout(location = 0) out vec4  outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv ;
layout(location = 2) in vec2 uvMesh ;
//layout(location = 2) in float scale;
layout(location = 3) in vec3 worldSpacePos;
layout(location = 4) in vec4 clipSpace;
//layout(location = 4) in float depth;


layout(set = 1, binding = 0) uniform sampler2D[2] u_Noise;
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


vec3 blend(vec4 texture1, float a1, vec4 texture2, float a2)
{
    return texture1.rgb * a1 + texture2.rgb * a2;
    //return a1 > a2 ? texture1.rgb  *a1 :  texture2.rgb *a2;
}

const float far = 10000;
 const float near = 10.00f;
 float linerizeDepth(float depth)
{
    float ndc = depth * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
    linearDepth/=far;
    return linearDepth;
}
    
void main() 
{

  
    vec2 scaledUvMesh = fract(uvMesh*3);
    
    
    vec4 noiseTex = texture(u_Noise[0],uv);
    vec4 snowTex = texture(u_Tex1,scaledUvMesh);
    vec4 groundTex = texture(u_Tex2,scaledUvMesh);
    vec4 stoneTex = texture(u_Tex3,scaledUvMesh);
    float currentVertexHeight  = color.x;
    float maxVertexHeight  = 5;
    float t = currentVertexHeight / maxVertexHeight;
  
    

    float step1 = maxVertexHeight * 0.20;
    float step2 = maxVertexHeight * 0.50;
    float step3 = maxVertexHeight * 0.75;
    vec4 blendedColor ;
    if (t < step1)
    {
        blendedColor=groundTex;
    }
    else if (t < step2)
    {
        float rand = random(uv);
        float t = norm(t,step2,step1);
        rand = norm(rand,0.2,-0.2);
        vec4 newColor = mix(groundTex,stoneTex,t);

        blendedColor= newColor;

    }
    else 
    {
        float t = norm(t,step3,step2);
        vec4 newColor = mix(stoneTex,snowTex,t);

        blendedColor= newColor;
    }





    float currentDepth = gl_FragCoord.z*(1/gl_FragCoord.w);
   // outColor.xyz = vec3(currentDepth,currentDepth,currentDepth);
  outColor.xyz =blendedColor.xyz;

 float linearDepth = linerizeDepth(gl_FragCoord.z);

//linearDepth = clipSpace.z/clipSpace.w;
if(linearDepth<gl_FragCoord.z)
{
    //outColor.xyz = vec3(linearDepth,linearDepth,linearDepth);
}

 //outColor.xyz= vec3(linearDepth,linearDepth,linearDepth);
 outColor.a = 1;
 gl_FragDepth = linearDepth;
   
}