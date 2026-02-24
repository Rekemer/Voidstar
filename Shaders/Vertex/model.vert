#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(set = 0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;
layout(location = 0) out vec2 out_uv;


layout(std430, set=0, binding=1) readonly buffer Objects {
    mat4 model[];
} objects;

void main()
{
    gl_Position = ubo.proj * ubo.view * objects.model[gl_InstanceIndex] * vec4(in_pos.xyz, 1.0);

    vec2 uv = vec2( in_uv.x, in_uv.y);
    out_uv = uv;
}  