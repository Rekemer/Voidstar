#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    
} ubo;

// vertexAttributes
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

// Instanced attributes
layout (location = 3) in vec3 instancePos;
layout (location = 4) in float instanceScale;
layout (location = 5) in int texIndex;

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

layout(location = 0) out vec2 uv ;
layout(location = 1) out vec4 color ;
layout(location = 2) out float scale ;
layout(location = 3) out vec4 worldSpacePos;
layout(location = 4) out float depth;



void main() 
{
    
    
	vec4 worldPos= vec4((in_pos*instanceScale)+instancePos,1.0);
	worldSpacePos = worldPos;
    depth = texIndex;
    scale=instanceScale;
	uv = in_uv;
	color = vec4(1,0,0,1);
	vec4 pos =	 worldPos;
	gl_Position = ubo.proj*ubo.view*pos;
}