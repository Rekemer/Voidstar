#version 450
layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 fragPos;
layout(location = 3) in float texIndex;
layout(location = 0) out vec4 outColor;
// remember it correlates with pageAmount on cpu
layout(set = 1, binding = 0) uniform sampler2D Text[3];

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
    float time;
    
} ubo;

void main() 
{
    vec2 modifiedUv = vec2(uv.x,1 - uv.y);
    float distance = length(ubo.playerPos.x - fragPos.x)/300.f;
    distance = clamp(distance, 0,1);
    float leftShadow = mix(distance,1,length(ubo.playerPos.x - 150)/300.f);
    leftShadow = clamp(leftShadow,0,1);
    leftShadow  = smoothstep(0,1,leftShadow);
    vec3 tex = texture(Text[int(texIndex)],modifiedUv).xyz;
    outColor = color;
    outColor = vec4(tex+ color.xyz,color.a);
    float leftX = 150*2.f;
    float width = 400;
    float centerPos = leftX + width;
    float tt = (length(centerPos - fragPos.x)+40)/(width/2.f);
    tt = clamp(0,1,tt);
    float centerShadow = smoothstep(0,1,tt);
    centerShadow = clamp(centerShadow, 0,1);
    outColor.xyz *= leftShadow * centerShadow ;
   // outColor.xy = uv;   outColor.z = 0;outColor.a = 1;
 
    
}