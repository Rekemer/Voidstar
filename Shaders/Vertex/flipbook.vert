#version 450
#extension GL_ARB_shader_draw_parameters : require
layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;


layout(std430, set = 0, binding = 1) buffer InstanceBuffer {
    mat4 worlds[];
};


layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;


struct Particle {
    vec4 time;
};


layout(std430, set = 1, binding=0) readonly buffer Particles {
    Particle p[];
} particles;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out float out_age;
layout(location = 2) out float out_lfietime;
layout(location = 3) out flat int out_frame;

float hash(uint x) {
    x += ( x << 10u ); x ^= ( x >>  6u );
    x += ( x <<  3u ); x ^= ( x >> 11u );
    x += ( x << 15u );
    return float(x) * (1.0 / 4294967296.0);
}

void main() 
{
    uint localIdx = gl_InstanceIndex - gl_BaseInstanceARB;
    vec4 worldPos = worlds[gl_InstanceIndex] * vec4(in_pos,1);

    float scaleX = length(worlds[gl_InstanceIndex][0].xyz);
    float scaleY = length(worlds[gl_InstanceIndex][1].xyz);

    vec3 worldCenter = worlds[gl_InstanceIndex][3].xyz;
    vec3 cameraRight = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    vec3 cameraUp    = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);
    
    vec3 billboardedPos = worldCenter.xyz 
                        + cameraRight * in_pos.x 
                        + cameraUp    * in_pos.y * 1.5;

    // 4. Transform to Clip Space
    gl_Position = ubo.proj * ubo.view * vec4(billboardedPos, 1.0);

    //vec4 clipSpace = ubo.proj * ubo.view * worldPos;
    //gl_Position = clipSpace;
    out_uv = in_uv;
    Particle p = particles.p[localIdx];
    out_age =p.time.x;
    out_lfietime = p.time.y; 
    float noiseOffset = hash(gl_InstanceIndex) * 10.0;
    int cols = 8;
    int rows = 8;
    int frameCount = cols * rows;
    float animFps = 30.0;  
    int frame = int(floor((out_age + noiseOffset) * animFps)) % frameCount; 
    out_frame = frame;

}