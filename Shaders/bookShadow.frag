#version 450



layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 fragPos;
layout(location = 0) out vec4 outColor;


layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
    float time;
    
} ubo;

layout(set = 1, binding = 0) uniform sampler2D u_Selected;
layout(set = 1, binding = 1) uniform sampler2D u_Noise1;

float scale = 100;





void main() 
{
    float distance = length(ubo.playerPos.xy - fragPos)/300.f;
    distance = clamp(distance, 0,1);
    outColor = vec4(vec3(distance*color),1);
}