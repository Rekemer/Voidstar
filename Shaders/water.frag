#version 450

layout(location = 0) out vec4  outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv ;
layout(location = 2) in vec2 uvMesh ;
layout(location = 3) in vec4 worldPos;
//layout(location = 2) in float scale;
//layout(location = 4) in float depth;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;

    float time;
    
} ubo;
layout (input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput inputDepth;

layout(set = 1, binding = 0) uniform sampler2D[2] u_Noise;
layout(set = 1, binding = 1) uniform sampler2D u_Tex1;
layout(set = 1, binding = 2) uniform sampler2D u_Tex2;
layout(set = 1, binding = 3) uniform sampler2D u_Tex3;
//layout(set = 1, binding = 4) uniform sampler2D u_WaterNormal;
//layout(set = 1, binding = 5) uniform sampler2D u_WaterNormal1;

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
 const float near = 0.01;

float linerizeDepth(float depth)
{
    float ndc = depth * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
    linearDepth/=far;
    return linearDepth;
}

void main() 
{
     float u_nearPlane = near;
     float u_farPlane = far;
    
    //Calculate the linear depth value
   // float linearDepth =linerizeDepth();

    // Calculate the view direction
    vec3 viewDir = normalize(worldPos.xyz - ubo.playerPos.xyz);

    // Calculate the distance from the camera to the fragment

      // Sample the depth texture to get the depth value of the fragment from the depth buffer
    float depthValue = subpassLoad(inputDepth).r;
    if (depthValue < gl_FragCoord.z)
    {
        discard;
    }
    // Calculate the linear depth value of the fragment from the depth buffer
    float linearDepthFromDepthBuffer = linerizeDepth(depthValue);

    // Apply shading based on the distance from the shoreline
    vec3 shorelineColor = vec3(0.0, 0.0, 0.0); // Dark color for the shoreline
    vec3 waterColor = vec3(0.0, 0.5, 1.0); // Color for the water

   
    // Calculate the distance between the current fragment and the fragment in the depth buffer
    linearDepthFromDepthBuffer*= far;
    float currentDepth = gl_FragCoord.z*(1/gl_FragCoord.w);
    currentDepth +=1 ;
    float distanceFromShore = linearDepthFromDepthBuffer-currentDepth;
    // Distance at which the shoreline effect starts (adjust as needed)
    float shorelineDistanceThreshold = 0.1;



    // Calculate the final color based on the distance from the shoreline
    vec3 finalColor = mix(waterColor, shorelineColor, smoothstep(0.0, shorelineDistanceThreshold, distanceFromShore));





    const float waterDepth =1.3;
    float tiling = 1;
    


    float speed = 30*ubo.time;
    float speed1 = -20*ubo.time;
    vec3 lightPos = vec3(3,12,-5);
   // vec4 waterNormal = texture(u_WaterNormal,uvMesh*tiling+speed);
    //vec4 waterNormal1 = texture(u_WaterNormal1,uvMesh*tiling+speed1);
    vec3 combinedTex= texture(u_Noise[1],uv).xyz;
   
    //vec3 lightDir = normalize(lightPos - worldPos.xyz);
    // vec3 lightDir = normalize(vec3(3,12,12));
    // vec3 norm = normalize(vec3(combinedTex));
    // float diff = max(dot(norm, lightDir), 0.0);
    // vec3 reflectDir = reflect(-lightDir, norm);
    // vec3 viewDir = normalize(ubo.playerPos.xyz - worldPos.xyz);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
    //outColor.xy =uvMesh;
    //outColor.z = 0; 
    //dist = color.z;
   // diff = length(ubo.playerPos.xyz - worldPos.xyz);
   //vec3 waterColor =  vec3(0.2,0.4,0.4);
   // vec3 diffuse  =  diff * waterColor;
     //vec3 finalColor= diffuse+spec;
    //if (currentDepth < gl_FragCoord.z)
    {
       // finalColor = vec3(1,0,1);
    }
   finalColor = vec3(distanceFromShore,distanceFromShore,distanceFromShore);  
   //finalColor =  subpassLoad(inputDepth,).xyz;  
    outColor.xyz = finalColor;
	outColor.a = 1;

    
   
}