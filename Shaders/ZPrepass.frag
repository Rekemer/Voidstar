#version 450

layout(location = 0) out vec4  outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv ;
layout(location = 2) in vec2 uvMesh ;
layout(location = 3) in vec3 worldPos;
//layout(location = 2) in float scale;
//layout(location = 4) in float depth;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;

    float time;
    
} ubo;

const float farPlane =10000.0;
void main() 
{
    vec4 screenPos = ubo.proj*ubo.view* vec4(worldPos,1);   
    gl_FragDepth = screenPos.z / screenPos.w;
}