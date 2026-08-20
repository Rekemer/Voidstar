#version 450

layout(location = 0) out vec4 color ;

layout(location = 0) in vec2 in_uv;

void main() 
{
    color = vec4(in_uv,0,1);
}