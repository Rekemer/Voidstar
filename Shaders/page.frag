#version 450
layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 fragPos;
layout(location = 0) out vec4 outColor;
//layout(set = 1, binding = 0) uniform sampler2D Text[8];


void main() 
{
    outColor = color;
}