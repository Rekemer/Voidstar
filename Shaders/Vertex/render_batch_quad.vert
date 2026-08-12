#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;
layout(set = 0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;
    
layout(location = 0) out vec4 out_color ;
void main()
{

    gl_Position = vec4(in_pos.xyz, 1.0);
    out_color = in_color;
}  