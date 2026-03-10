#version 450

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(std430, set = 0, binding = 1) readonly buffer InstanceMatrices
{
    mat4 model[];
} instances;


layout(location = 0) in vec3 in_pos;

layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_worldPos;

void main() 
{
    mat4 model = instances.model[gl_InstanceIndex];
    vec4 worldPos = model * vec4(in_pos,1);
    vec4 clipSpace = ubo.proj * ubo.view * worldPos ;
    gl_Position = clipSpace;
    out_uv = in_uv;
    out_worldPos = worldPos .xyz;
 

}