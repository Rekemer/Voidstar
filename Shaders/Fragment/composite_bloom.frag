#version 450

layout(location = 0) in vec2 v_TexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D u_Scene; 
layout(set = 1, binding = 1) uniform sampler2D u_Bloom;  

void main() {
    vec3 sceneColor = texture(u_Scene, v_TexCoord).rgb;
    vec3 bloomColor = texture(u_Bloom, v_TexCoord).rgb;

    float bloomIntensity = 0.05;
    vec3 result = sceneColor + (bloomColor * bloomIntensity);

    vec3 mapped = result / (result + vec3(1.0));

    mapped = pow(mapped, vec3(1.0 / 2.2));

    outColor = vec4(mapped, 1.0);
}