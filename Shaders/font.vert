#version 450


layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(set = 0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
    float time;
} ubo;
layout(location = 0) out vec2 TexCoords;

void main()
{
    gl_Position = ubo.proj * vec4(in_pos.xy, 0.0, 1.0);
    TexCoords = in_uv;
}  