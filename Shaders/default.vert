#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
} ubo;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 0) out vec2 out_uv;



void main() 
{
    
    vec3 instanceScale = vec3(100);
    vec3 instancePos = vec3(0);    
	vec4 worldPos= ubo.proj*ubo.view* vec4((in_pos*instanceScale)+instancePos,1.0);
	out_uv = vec2(1-in_uv.x,in_uv.y);
    gl_Position = worldPos;

}