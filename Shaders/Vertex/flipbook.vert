#version 450

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;





layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;


struct Particle {
    vec4 pos;
    vec4 time;
};


layout(std430, set = 1, binding=0) readonly buffer Particles {
    Particle p[];
} particles;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out float out_age;
layout(location = 2) out float out_lfietime;

void main() 
{
    
    vec3 worldPos = in_pos + particles.p[gl_InstanceIndex].pos.xyz;
    vec4 clipSpace = ubo.proj * ubo.view * vec4(worldPos,1);
    gl_Position = clipSpace;
    out_uv = in_uv;
    Particle p = particles.p[gl_InstanceIndex];
    out_age =p.time.x;
    out_lfietime = p.time.y; 

}