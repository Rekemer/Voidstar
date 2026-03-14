#version 450

layout(location = 0) in vec2 v_TexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D u_Scene; 
layout(set = 1, binding = 1) uniform sampler2D u_Bloom;  


vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 sceneColor = texture(u_Scene, v_TexCoord).rgb;
    vec3 bloomColor = texture(u_Bloom, v_TexCoord).rgb;

    float bloomIntensity = .2;
    vec3 result = sceneColor + (bloomColor * bloomIntensity);

    float exposure = 1.2/3;
    result *= exposure;
    vec3 mapped = ACESFilm(result);

    mapped = pow(mapped, vec3(1.0 / 2.2));
    outColor = vec4(mapped, 1.0);
}