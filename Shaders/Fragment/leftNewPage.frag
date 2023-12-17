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

vec2 invLerp(vec2 a, vec2 b, vec2 v)
{
    return (v - a)/ (b-a);
}
float inverse_smoothstep(float x) {
  return 0.5 - sin(asin(1.0 - 2.0 * x) / 3.0);
}
void main() 
{
    vec2 modifiedUv = vec2(uv.x,1 - uv.y);
    vec3 tex = texture(Text[int(texIndex)],modifiedUv).xyz;
    outColor = color;
    outColor = vec4(tex+ color.xyz,color.a);
    float leftX = 150*2.f;
    float leftY = 200;
    float width = 400;
    float centerPos = leftX + width;
    float distance = (length(ubo.playerPos.xy - vec2(leftX,leftY)))/(centerPos+width);
    distance = clamp(distance,0,1);
    distance = smoothstep(0,1,distance);
    float tt = (length(centerPos - fragPos.x )+40+10000*distance)/(width/2);
    tt = clamp(tt,0,1);
    vec2 ttt = invLerp(vec2(leftX,leftY),vec2(leftX + width*2,leftY), fragPos);
    ttt = clamp(ttt,vec2(0),vec2(1));
    float centerShadow = smoothstep(0,1,tt) ;
    outColor.xyz *= centerShadow ;
    outColor.xyz *= (1- distance);
 
    
}