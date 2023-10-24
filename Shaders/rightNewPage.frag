#version 450



layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 fragPos;
layout(location = 3) in float texIndex;
layout(location = 0) out vec4 outColor;
layout(set = 1, binding = 0) uniform sampler2D Text[4];

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
    float time;
    
} ubo;

layout(set = 1, binding = 0) uniform sampler2D u_Selected;
layout(set = 1, binding = 1) uniform sampler2D u_Noise1;

float scale = 100;





void main() 
{
    float leftX = 150;
    float width = 250;
    float centerPos = leftX + width;
    
    float distance = length(ubo.playerPos.x - fragPos.x)/300.f;
    distance = clamp(distance, 0,1);
    float rightPageShadow = smoothstep(0,1,distance);
    vec2 modifiedUv = vec2(uv.x,1 - uv.y);
    vec3 tex = texture(Text[int(texIndex)],modifiedUv).xyz;
    outColor = color;
    outColor = vec4(tex+ color.xyz,color.a);
    
    float tt = (length(centerPos - fragPos.x)+40)/(width/2.f);
    tt = clamp(tt,0,1);
    float centerShadow = smoothstep(0,1,tt);
    centerShadow = clamp(centerShadow, 0,1);    
    outColor.xyz *= rightPageShadow*centerShadow;

    //outColor = vec4(vec3(distance*color),1);
}