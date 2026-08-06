#version 450

layout(location = 0)  in vec2 in_uv;
layout(location = 1)  in vec3 in_fragPos;
layout(location = 2)  in mat3 in_TBN;

layout(location = 0)  out vec4 color ;

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
    vec4 tex = texture(u_Albedo,vec2(in_uv));
    vec3 normal = texture(u_Normal,vec2(in_uv)).xyz;
    // normal should be in range from -1 to 1
    normal = normalize(normal * 2.0 - 1.0);
    normal = in_TBN * normal;
    vec3  lightPos = vec3( 0,-10,0);
    vec3  lightDir = normalize(lightPos - in_fragPos);
    float lit = max(dot(normal, lightDir), 0.0);

    color = vec4(lightDir,1);
    color = vec4(vec3(lit),1);
    color = vec4(tex.xyz * lit,1);
}