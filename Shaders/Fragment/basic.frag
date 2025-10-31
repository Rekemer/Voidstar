#version 450

layout(location = 0) out vec4 color ;

layout(location = 0) in vec2 out_uv;

void main() 
{
    color = vec4(1,0,1,1);
    color = vec4( out_uv,0,1);
}