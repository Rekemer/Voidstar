#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent;

layout(set = 0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;
layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_fragPos;
layout(location = 2) out mat3 out_TBN;


layout(std430, set=0, binding=1) readonly buffer Objects {
    mat4 model[];
} objects;

void main()
{
    mat4 world = objects.model[gl_InstanceIndex];
    gl_Position = ubo.proj * ubo.view * world * vec4(in_pos.xyz, 1.0);

    vec2 uv = vec2( in_uv.x, in_uv.y);
    // do on CPU instead instead of on GPU on every vertex
    mat3 normalMatrix = mat3(transpose(inverse(world)));

    vec3 T = normalize(normalMatrix * in_tangent.xyz);
    vec3 N = normalize(normalMatrix * in_normal);
    vec3 B = cross(N,T) * in_tangent.w;
    out_TBN = mat3(T, B, N);

    out_fragPos = vec3(world * vec4(in_pos.xyz, 1.0)); 
    out_uv = uv;
}  