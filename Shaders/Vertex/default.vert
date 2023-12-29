#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;
layout(location = 3) in float in_texIndex;
layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;
layout(location = 2) out vec2 out_position;
layout(location = 3) out float out_texIndex;



void main() 
{
    
    vec3 instanceScale = vec3(100,100,1);
    vec3 instancePos = vec3(100,500,0); 
    vec3 worldPos = in_pos;
    vec4 clipSpace = ubo.proj * vec4(worldPos,1);
	out_uv = vec2(in_uv.x,in_uv.y);
    out_position = worldPos.xy;
    gl_Position = clipSpace;
    out_color = in_color;
    out_texIndex = in_texIndex;

}