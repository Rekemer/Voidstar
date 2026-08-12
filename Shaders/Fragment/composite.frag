#version 450
layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 color;
layout(set = 1, binding = 0) uniform sampler2D u_TopLayer;
void main()
{
    color = vec4(1,1,1,1);
    color = texture(u_TopLayer, in_uv);
}