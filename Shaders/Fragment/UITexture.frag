#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec4 in_color ;
layout(location = 1) in vec2 in_uv;


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;



layout(set = 1, binding = 0) uniform sampler2D u_Texture;


void main() 
{
    vec4 tex = texture(u_Texture,vec2(in_uv));
    color = vec4(tex.xyzw) + vec4(in_color.xyz * (1 - tex.w), in_color.w);
    //color = vec4(1,0,1,1);
}