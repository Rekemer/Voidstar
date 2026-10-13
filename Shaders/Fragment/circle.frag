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
    vec2 centeredUV = (in_uv * 2) - 1;
    float radius =  1 - length(centeredUV);
    radius = smoothstep(0.0,0.2,radius);

    vec3 circle = vec3(radius);
    color = vec4(vec3(in_color.xyz),radius);
}