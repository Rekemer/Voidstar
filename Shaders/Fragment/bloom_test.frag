// #version 450
// layout(location = 0) out vec4 normalColor;
// layout(location = 1) out vec4 brightColor;
// void main() {
//    vec3 color = vec3(12,2,1);
//    // Use a luminosity check or just check if RGB > 1.0
//    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
//    if(brightness > 1.0)
//        brightColor = vec4(color.rgb, 1.0);
//    else
//        normalColor = vec4(0.0, 1.0, 0.0, 1.0);
// }
#version 450
layout(location = 0) out vec4 normalColor; // Your actual game scene
layout(location = 1) out vec4 brightColor; // The bloom starting point


void main() {
   vec3 color = vec3(12,2,1);

    float threshold = 2.0; 
    float knee = 0.5;
    // -------------------------------------
    //normalColor = vec4(color, 1.0);

    float brightness = max(color.r, max(color.g, color.b));

    float soft = brightness - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee + 0.00001);
    
    float contribution = max(soft, brightness - threshold);
    contribution /= max(brightness, 0.00001);

    brightColor = vec4(color * contribution, 1.0);
}