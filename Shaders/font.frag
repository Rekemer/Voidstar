#version 450
layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4  outColor;

layout(set = 1, binding = 0) uniform sampler2D u_TextAtlas;

void main()
{    
    vec3 fontColor = vec3(1,1,1);
    vec4 textColor = vec4(fontColor, texture(u_TextAtlas, TexCoords).r);
    outColor = vec4(1,0,1,1);
    outColor = vec4(textColor);
    //outColor = vec4( TexCoords,0,1);
}  