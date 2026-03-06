#version 450

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(location = 0) in vec3 in_pos;

layout(location = 1) in vec2 in_uv;

layout(location = 2) in float age ;
layout(location = 3) in float lifetime ;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out float out_age;
layout(location = 2) out float out_lfietime;

void main() 
{
    
    vec3 worldPos = in_pos ;
    vec4 clipSpace = ubo.proj * ubo.view * vec4(worldPos,1);
    gl_Position = clipSpace;
    out_uv = in_uv;
    out_age =age;
    out_lfietime=lifetime; 

}