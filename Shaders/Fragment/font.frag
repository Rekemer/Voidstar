#version 450
layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 TexCoords;
layout(location = 0) out vec4  outColor;

layout(set = 1, binding = 0) uniform sampler2D u_Atlas;

void main()
{    
    vec4 textColor = vec4(in_color.xyz, texture(u_Atlas, TexCoords).r);
    outColor = vec4(1,0,1,1);
    outColor = vec4(textColor);
}  