#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    
} ubo;
layout(set = 1, binding = 0) uniform sampler2D u_Tex;
layout(set=2,binding = 1) uniform NoiseData {

    float frequence ;
	float amplitude ;
	float octaves ;
	float textureHeight ;
	float textureWidth ;
	float multipler ;
    
} noiseData;
// vertexAttributes
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;
layout(location = 2) in float in_noise;
layout(location = 3) in vec2 in_uv;

// Instanced attributes
layout (location = 4) in vec3 instancePos;
layout (location = 5) in vec4 instanceEdges;
layout (location = 6) in float instanceScale;
vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec4 color ;
layout(location = 1) out vec2 uv ;
layout(location = 2) out vec4 edges;

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
    
    
// Constants
    const int gridSize = 5000;
    const float cellSize = 1.0 / float(gridSize);
	//vec4 worldPos= ubo.model * vec4((in_pos+instancePos)*instanceScale,1.0);
    float tilesAmount  = 2.;
	vec4 worldPos= vec4((in_pos*instanceScale)+instancePos,1.0);
	vec4 worldSpacePos = worldPos;
	
	color = in_color;
    
    


    
    
   
    
    
    
    vec2 cellOffset;





    vec2 cell = floor(worldPos.xz * gridSize);
  
    vec2 playerPos= vec2(0,0);
    vec2 diff = playerPos - worldPos.xz;

    vec2 newUv = GetUvs(worldPos.xz, gridSize, in_uv,gridSize/(instanceScale) );
	uv = newUv;

    float noiseValue = texture(u_Tex,newUv).x;
    worldPos.y = abs(noiseValue)*noiseData.multipler ;
	color.xyz =vec3( worldPos.y, noiseData.multipler, worldPos.y) ;
   // color.xyz = vec3(newUv,0.);
    //color.xyz = vec3(noiseValue,noiseValue,noiseValue);
    //color.xyz= vec3( randomGradient(cell),0.);
	//vec4 pos =	ubo.proj *  ubo.view * worldPos;
	vec4 pos =	 worldPos;
	gl_Position = pos;
    edges = instanceEdges;
}