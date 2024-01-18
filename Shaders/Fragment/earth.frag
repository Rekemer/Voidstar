#version 450

layout(location = 0) out vec4  outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;





void main() 
{
    outColor = color;  
    outColor = vec4(uv,0,1);
}