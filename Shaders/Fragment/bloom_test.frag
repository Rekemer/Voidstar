#version 450
layout(location = 0) out vec4 normalColor;
layout(location = 1) out vec4 brightColor;
void main() {
    vec3 color = vec3(12,0,1);
    // Use a luminosity check or just check if RGB > 1.0
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightColor = vec4(color.rgb, 1.0);
    else
        normalColor = vec4(0.0, 1.0, 0.0, 1.0);
}