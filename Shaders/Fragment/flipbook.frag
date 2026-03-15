#version 450
layout(location = 0) out vec4 normalColor;
layout(location = 1) out vec4 brightColor;


layout(location = 0) in vec2 out_uv;
layout(location = 1) in float out_age;
layout(location = 2) in float out_lifetime;
layout(location = 3) in flat int out_frame;

layout(set = 1, binding = 1) uniform sampler2D u_Texture;

layout(set = 1, binding = 2) readonly buffer FireData {
    vec4 colorLow;   
    vec4 colorMid;   
    vec4 colorHigh;  
    float intensity; 
} fire;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}


void main()
{
    // int cols = 8;
    // int rows = 8;
    // int frameCount = cols * rows;

    // float t = clamp(out_age / out_lifetime, 0.0, 1.0);              

    
    // int frame = out_frame;
    // int fx = frame % cols;
    // int fy = frame / cols;
    // fy = (rows - 1) - fy;
  

    // vec2 uv = out_uv;
    // uv.y = 1.0 - uv.y;
    // vec2 cell = vec2(1.0 / float(cols), 1.0 / float(rows));
    // vec2 uv_fb = uv * cell + vec2(float(fx), float(fy)) * cell;

    // float noise = random(out_uv + (out_age * 0.5));
    // //uv_fb += (noise - 0.5) * 0.004; 

    // vec4 tex = texture(u_Texture, uv_fb);
    // float brightness = dot(tex.rrr, vec3(0.2126, 0.7152, 0.0722));
    // // Set alpha based on how bright the pixel is
    // tex.a = clamp(brightness * 2.0, 0.0, 1.0);
    // vec4 color = tex;

    
    // float brightnessOverall = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));

   


    // normalColor = color;
    // if(brightnessOverall> 1.0)
    //     brightColor = vec4(color);
    // else    
    //     brightColor = vec4(0,0,0, color.a);


    // --- Flipbook UV Logic ---
    int cols = 8;
    int rows = 8;
    int fx = out_frame % cols;
    int fy = (rows - 1) - (out_frame / cols);

    vec2 uv = out_uv;
    uv.y = 1.0 - uv.y;
    vec2 cell = vec2(1.0 / float(cols), 1.0 / float(rows));
    vec2 uv_fb = uv * cell + vec2(float(fx), float(fy)) * cell;

    vec4 tex = texture(u_Texture, uv_fb);
    tex.rgb *= 1.75;
    float mask = tex.r; 

    // --- Using the Uniform Colors ---
    vec3 fireColor = mix(fire.colorLow.rgb, fire.colorMid.rgb, smoothstep(0.1, 0.5, mask));
    fireColor = mix(fireColor, fire.colorHigh.rgb, smoothstep(0.6, 0.9, mask));

    // Apply intensity to push into HDR for bloom
    vec3 finalRGB = fireColor * mask * fire.intensity;

    // Use smoothstep on alpha to remove the "box" edges
    float alpha = smoothstep(0.05, 0.2, mask);

    normalColor = vec4(finalRGB, alpha);

    // Bloom Extraction
    float luma = dot(finalRGB, vec3(0.2126, 0.7152, 0.0722));
    if(luma > 1.0) {
        brightColor = vec4(finalRGB, alpha);
    } else {
        brightColor = vec4(0.0); // Transparent black, no artifacts!
    }
    
}