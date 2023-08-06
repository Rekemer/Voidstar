// tessellation evaluation shader
#version 450


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
    
} ubo;
layout(set = 1, binding = 0) uniform sampler2D[2] u_Tex;
layout(set=2,binding = 2) uniform NoiseData {

    float frequence ;
	float amplitude ;
	float octaves ;
	float textureHeight ;
	float textureWidth ;
	float multipler ;
        float normalStrength ;
	float waterScale;
    
} noiseData;
layout(location = 0) in vec4[] inColor ;
layout(location = 1) in vec2[] ivUv ;
layout(location = 2) in vec2[] inUvMesh ;
layout(location = 3) in vec4[] worldPos ;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUv;
layout(location = 2) out vec2 outUvMesh;
layout(location = 3) out vec4 worldSpacePos;
layout (quads) in;


float norm(float val, float max,float min)
{
     return (val- min) / (max - min);
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

    float gridSize = 5000;
    float tileWidth = length(gl_in[0].gl_Position - gl_in[1].gl_Position);
   
    vec2 newUv = GetUvs(p.xz,gridSize, vec2(u,v),gridSize / (tileWidth));

    float noiseValue = texture(u_Tex[1],newUv).x;
   
     vec4 color = mix(
        mix(inColor[0], inColor[1], barycentricCoord.x),
        mix(inColor[3], inColor[2], barycentricCoord.x),
        barycentricCoord.z
    );

    vec3 offset = texture(u_Tex[1],newUv).xyz;
    p.xyz+=offset *noiseData.normalStrength *noiseData.multipler  ; 
     outColor = vec4(newUv,0,1);
     outColor =color;
     outUv = newUv;
     outUvMesh = gl_TessCoord.xy;
     worldSpacePos = p;
    gl_Position = ubo.proj*ubo.view*p;
}