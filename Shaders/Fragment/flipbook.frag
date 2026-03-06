#version 450

layout(location = 0) out vec4 color ;
layout(location = 0) in vec2 out_uv;
layout(location = 1) in float out_frame;

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;



layout(set = 1, binding = 0) uniform sampler2D u_Texture;
//layout(set = 1, binding = 1) uniform sampler2D u_Texture1;
//layout(set = 2, binding = 0) uniform sampler2D u_Texture1;


void main() 
{
    //vec4 tex1 = texture(u_Texture1,vec2(out_uv));
    //color = vec4( out_uv,0,1);
    //float fact = (sin(ubo.time * 1000) + 1)/ 2;
    //color = vec4( tex.xyz + tex1.xyz,1 * fact);
    vec3 frame = vec3(out_frame,out_frame,out_frame);
    vec4 tex = texture(u_Texture,vec2(out_uv));
    color = vec4(frame.xyz,1);
}