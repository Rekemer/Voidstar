#version 450
layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;


layout(set = 1, binding = 0) uniform sampler2D u_Scene;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
void main ()
{
     vec4 tex = texture(u_Scene,uv) ;
     outColor = vec4(uv,0,1);
     outColor = vec4(tex.xyz,1);
}