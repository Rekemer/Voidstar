#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

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

void main() 
{
	//vec4 pos =	ubo.proj *  ubo.view * ubo.model * vec4(in_pos,1.0);
	vec4 pos =	 ubo.proj*ubo.view *vec4(in_pos,1.0);
	//gl_Position = vec4(in_pos,1.0);
	gl_Position = pos;
	uv = in_uv;
	uv = vec2(in_uv.x,in_uv.y);
	color = in_color;
	//uv.x=1-uv.x;
	//uv.y=1-uv.y;
		
}