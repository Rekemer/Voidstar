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
const vec2 screen_corners[6] = vec2[](
    vec2(-1.0, -1.0), // left top
    vec2(-1.0,  1.0), // left bottom
    vec2( 1.0,  1.0), // right bottom
    vec2( 1.0,  1.0), // right bottom
    vec2( 1.0, -1.0), // right top
    vec2(-1.0, -1.0)  // left top
);

void main()
{
    gl_Position = ubo.proj * vec4(in_pos.xy, 0.0, 1.0);
    vec4 pos =  vec4(screen_corners[gl_VertexIndex],0.3,1.0);
   // gl_Position =  vec4(screen_corners[gl_VertexIndex],0.3,1.0);
    vec2 uv = (pos.xy + 1.0) * 0.5;
    //uv.y = 1 - uv.y;
    TexCoords = in_uv;
}  