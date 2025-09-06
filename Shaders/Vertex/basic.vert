#version 450

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(location = 0) in vec3 in_pos;

void main() 
{
    
    vec3 worldPos = in_pos;
    vec4 clipSpace = ubo.proj * vec4(worldPos,1);
    gl_Position = clipSpace;

}