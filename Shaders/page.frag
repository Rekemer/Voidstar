#version 450
layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 fragPos;
layout(location = 3) in float texIndex;
layout(location = 0) out vec4 outColor;
layout(set = 1, binding = 0) uniform sampler2D Text[4];


void main() 
{
    vec2 modifiedUv = vec2(uv.x,1 - uv.y);
    vec3 tex = texture(Text[int(texIndex)],modifiedUv).xyz;
    outColor = color;
    outColor = vec4(tex+ color.xyz,color.a);
   // outColor.xy = uv;   outColor.z = 0;outColor.a = 1;
 
    
}