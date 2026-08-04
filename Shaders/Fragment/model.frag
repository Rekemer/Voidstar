#version 450

layout(location = 0) out vec4 color ;
layout(location = 0) in vec2 out_uv;


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;



layout(set = 1, binding = 0) uniform sampler2D u_Albedo;
layout(set = 1, binding = 1) uniform sampler2D u_Normal;
layout(set = 1, binding = 2) uniform sampler2D u_Metallic;


void main() 
{
    vec4 tex = texture(u_Albedo,vec2(out_uv));
    vec3 normal = texture(u_Normal,vec2(out_uv)).xyz;
    normal = normalize(normal * 2.0 - 1.0);
    
    color = vec4(tex.xyz,1);
}