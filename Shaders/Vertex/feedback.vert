#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)


layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout( location = 0) in vec3 in_pos;
layout( location = 1) in vec2 in_uv;
layout( location = 2) in vec4 in_color;
layout( location = 3) in float in_texIndex;



layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_uv;



void main() 
{

    mat4 rot;
    rot[0] = vec4(1,0,0,0);
    rot[1] = vec4(0,1,0,0);
    rot[2] = vec4(0,0,1,0);
    rot[3] = vec4(0,0,0,1);
    vec4 clipSpace = ubo.proj * ubo.view* vec4(in_pos,1);
    //clipSpace /=clipSpace.w;
    
    gl_Position = clipSpace;
    out_uv = in_uv;
}