#version 450
layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(location = 0) in vec2 uv ;
layout(location = 0) out vec4 color ;
layout(set = 1, binding = 0) uniform sampler2D u_Tex;

void main() 
{
    color = texture(u_Tex,uv);
    //color = vec4(1,0,1,1);
}