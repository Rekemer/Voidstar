#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec4 in_color ;
layout(location = 1) in vec2 in_uv;


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;





void main() 
{
    float r = length(in_uv * 2.0 - 1.0);         

    const float thickness = 0.25;                 
    float inner = 1.0 - thickness;                

    float w = length(vec2(dFdx(r), dFdy(r)));     

    float outerAlpha = clamp((1.0 - r) / w + 0.5, 0.0, 1.0);   
    float innerAlpha = clamp((r - inner) / w + 0.5, 0.0, 1.0); 

    float alpha = outerAlpha * innerAlpha;

    color = vec4(in_color.xyz, alpha);           
}