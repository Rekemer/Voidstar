#version 450

layout(location = 0) in vec3 in_pos;
layout(set = 0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(std430, set=0, binding=1) readonly buffer Objects {
    mat4 model[];
} objects;

void main()
{
    mat4 world = objects.model[gl_InstanceIndex];
    gl_Position = ubo.proj * world * vec4(in_pos.xyz, 1.0);
}  