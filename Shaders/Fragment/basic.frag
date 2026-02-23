#version 450

layout(location = 0) out vec4 color ;

layout(location = 0) in vec2 in_uv;

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

void main() 
{
    color = vec4(1,0,1,1);
    float fact = (sin(ubo.time * 1000) + 1)/ 2;
    color = vec4( in_uv,0,1) ;

}