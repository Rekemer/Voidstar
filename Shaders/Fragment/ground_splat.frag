#version 450

layout(location = 0) in vec2 out_uv;
layout(location = 1) in vec3 out_worldPos;

layout(location = 0) out vec4 normalColor;
layout(location = 1) out vec4 brightColor;

layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;


layout(set = 1, binding = 0) uniform sampler2D u_Texture; 
layout(set = 1, binding = 1) uniform sampler2D u_Grid;         
layout(set = 1, binding = 2) uniform sampler2D u_Noise[4];
layout(set = 1, binding = 3) readonly buffer FireData {
    vec4 colorLow;   
    vec4 colorMid;   
    vec4 colorHigh;  
    float intensity; 
} fire;

float sampleFieldBilinear(vec2 uv)
{
    vec2 texSize = textureSize(u_Grid,0).xy;
    // uv in [0,1]
    vec2 texelPos = uv * texSize - vec2(0.5);
    vec2 base = floor(texelPos);
    vec2 f = fract(texelPos);

    vec2 uv00 = (base + vec2(0.5, 0.5)) / texSize;
    vec2 uv10 = (base + vec2(1.5, 0.5)) / texSize;
    vec2 uv01 = (base + vec2(0.5, 1.5)) / texSize;
    vec2 uv11 = (base + vec2(1.5, 1.5)) / texSize;

    float a = texture(u_Grid, uv00).r;
    float b = texture(u_Grid, uv10).r;
    float c = texture(u_Grid, uv01).r;
    float d = texture(u_Grid, uv11).r;

    float x0 = mix(a, b, f.x);
    float x1 = mix(c, d, f.x);
    return mix(x0, x1, f.y);
}

void main() 
{
    // --- 1. COORDINATE WARPING ---
    // float warp = texture(u_Noise[0], out_uv * 1.5 + ubo.time * 0.1).r;
    // vec2 warpedUV = out_uv + (warp - 0.5) * 0.05;

    // // --- 2. MULTI-SCALE NOISE ---
    // float n1 = texture(u_Noise[1], warpedUV * 2.0 - vec2(0, ubo.time * 0.2)).r; // Large flow
    // float n2 = texture(u_Noise[2], warpedUV * 5.0 + vec2(ubo.time * 0.3, 0)).r; // Mid structure
    // float n3 = texture(u_Noise[3], out_uv * 15.0 + ubo.time * 1.5).r;           // High-speed sparks

    // // --- 3. FIELD & TARGETED MASKING ---
    // float field = sampleFieldBilinear(warpedUV);
    
    // // Tweak: Veins only exist where the field is strong
    // // This removes them from the "cold" areas but keeps them in the "hot" ones
    // float targetedVeins = pow(n2 * n3, 2.5) * field * 40.0;
    // targetedVeins = step(0,1,targetedVeins);
    // // Base body of the fire (the "painterly" part)
    // float baseFire = smoothstep(0.15, 0.7, field * n1 * 1.8);

    // // Combine: Add the sharp veins onto the smooth base
    // float fireMask = clamp(baseFire + targetedVeins, 0.0, 1.0);

    // // --- 4. COLOR & CHAR ---
    // // Transition based on the field (heat)
    // vec3 fireColor = mix(fire.colorLow.rgb, fire.colorMid.rgb, smoothstep(0.1, 0.5, field));
    // fireColor = mix(fireColor, fire.colorHigh.rgb, smoothstep(0.6, 0.9, field));

    // vec3 stone = texture(u_Texture, out_uv).rgb;
    
    // // Char/Burn the ground: turn it black where the field is active
    // float charMask = smoothstep(0.0, 0.6, field);
    // vec3 baseColor = stone * (1.0 - charMask * 0.92);

    // // Final composition
    // vec3 finalFire = fireColor * fireMask * fire.intensity;
    // vec3 finalRGB = baseColor + finalFire;

    // // --- 5. OUTPUTS ---
    // normalColor = vec4(vec3(targetedVeins), 1.0);

    // // Bloom Extraction
    // float luma = dot(finalFire, vec3(0.2126, 0.7152, 0.0722));
    // if(luma > 1.0)
    //     brightColor = vec4(finalFire, 1.0);
    // else
    //     // Zero alpha prevents the black box artifacts
    //     brightColor = vec4(0.0, 0.0, 0.0, 0.0);


// --- 1. COORDINATE WARPING (The "Licks") ---
    // Use Noise 0 to create the macro-distortion of the fire shape
    // --- 1. THE SWIRL (Anti-Diagonal) ---
    // Instead of just scrolling, we rotate the UVs slightly over time
    float angle = ubo.time * 1.5;
    mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    
    // Warp the coordinates using Noise 0 to create organic "licking"
   vec2 warpScroll = vec2(0.0, ubo.time * 0.4); 
    
    // Optional: Add a tiny bit of sine oscillation so it "sways" in place
    warpScroll.x = sin(ubo.time * 2.0) * 0.02;

    float warp = texture(u_Noise[0], out_uv * 1.5 + warpScroll).r;
    vec2 warpedUV = out_uv + (warp - 0.5) * 0.08;

    // --- 2. MULTI-DIRECTIONAL NOISE ---
    // n1 moves Up/Right, n2 moves Down/Left. This cancels out the "streak"
        float n1 = texture(u_Noise[1], warpedUV * 2.5 + vec2(ubo.time * 13.1, ubo.time * 12.1)).r;
    float n2 = texture(u_Noise[2], warpedUV * 4.0 - vec2(ubo.time * 12.15, -ubo.time * 13.1)).r;
    
    // Use Noise 3 as a static "Grain" to break the diagonal lines
    float n3 = texture(u_Noise[3], out_uv * 20.0).r;

   // --- 3. THE "SPARKY" DENSITY MASK ---
    float field = sampleFieldBilinear(warpedUV);
    
    // Internal Cracks (Sharp & Deep)
    float internalCracks = pow(n1 * n2 * 2.2, 3.8); 
    
    // THE SPARKS: Use the high-frequency n3 noise
    // We only want sparks near the edges of the field
    float sparks = pow(n3, 5.0) * step(0.1, field);
    
    // Combine: Add the sparks to the cracked density
    float fireDensity = (field * internalCracks) + (sparks * 0.4);

    // Sharpen the final mask
    float fireMask = smoothstep(0.05, 0.6, fireDensity);

    // --- 4. COLOR: TIGHTENING THE HEAT ---
    // Push the 'white-hot' even further back to keep the rich oranges
    vec3 fireColor = mix(fire.colorLow.rgb, fire.colorMid.rgb, smoothstep(0.05, 0.45, fireDensity));
    
    // ONLY the absolute peaks (sparks and center veins) get white
    fireColor = mix(fireColor, fire.colorHigh.rgb, smoothstep(0.85, 0.98, fireDensity));

    // --- 5. COMPOSITION ---
    vec3 stone = texture(u_Texture, out_uv * 5.0).rgb;
    
    // Charring: Make sure the charring follows the FIELD, not the cracks
    float charStrength = smoothstep(0.0, 0.7, field);
    vec3 finalStone = stone * (1.0 - charStrength * 0.9);

    vec3 finalFire = fireColor * fireMask * fire.intensity;
    normalColor = vec4(finalStone + finalFire, 1.0);
    
    // Bloom
    float luma = dot(finalFire, vec3(0.2126, 0.7152, 0.0722));
    if(luma > 1.0) {
        brightColor = vec4(finalFire, 1.0);
    } else {
        brightColor = vec4(0.0, 0.0, 0.0, 0.0);
    }


// float noiseVal = texture(u_Noise[0], fract(out_uv * 3.0 )).r;
//     vec2 noiseVec = vec2(noiseVal) * 2.0 - 1.0;
//     vec2 distortedUV = out_uv + noiseVec * 0.008;

//     float field = sampleFieldBilinear(distortedUV);
//     float baseCoverage = smoothstep(0.2, 0.6, field);

//     float edgeBand = 1.0 - abs(baseCoverage * 2.0 - 1.0);

//     float shapedField = field + (noiseVal - 0.5) * 0.06 * edgeBand;
//     float coverage = smoothstep(0.2, 0.4, shapedField);

//     float noiseTime = texture(u_Noise[2], out_uv * 3.0 + ubo.time * 10.2).r;
//     float n1 = texture(u_Noise[0], out_uv * 4.0 + vec2(ubo.time * 10.0, 0)).r;
//     float n2 = texture(u_Noise[2], out_uv * 3.0 + vec2(0, ubo.time * 9.15)).r;
//     float noise = mix(n1, n2, 0.2);
//     float flicker = smoothstep(0.4, 0.6, noise);
//     vec3 coldColor = vec3(0.1, 0.0, 0.0);    
//     vec3 midColor  = vec3(0.9, 0.2, 0.0);    
//     vec3 hotColor  = vec3(1.0, 0.9, 0.3);    
    
//     float erodedField = field - (noise * 0.2);
//     vec3 finalColor = mix(coldColor, midColor, smoothstep(0.1, 0.5, erodedField));
//     finalColor = mix(finalColor, hotColor, smoothstep(0.6, 0.9, erodedField));

    
    
//     normalColor = vec4( hotColor * vec3(coverage) * flicker, 1.0);

//     float luma = dot(finalColor, vec3(0.2126, 0.7152, 0.0722));
// if(luma > 1.0)
//     brightColor = vec4(finalColor, 1.0);
// else
//     brightColor = vec4(0.0, 0.0, 0.0, 0.0); // Fixes the black quads

}