#version 450
layout(location = 0) out vec4 color;
layout(location = 0) in vec2 out_uv;
layout(location = 1) in float out_age;
layout(location = 2) in float out_lifetime;

layout(set = 1, binding = 1) uniform sampler2D u_Texture;

void main()
{
    int cols = 10;
    int rows = 6;
    int frameCount = cols * rows;

    float t = clamp(out_age / out_lifetime, 0.0, 1.0);
   float animFps = 40.0;                 // try 12..30
    int frame = int(floor(out_age * animFps)) % frameCount;
    int fx = frame % cols;
    int fy = frame / cols;
    fy = (rows - 1) - fy;
  

    vec2 uv = out_uv;
    uv.y = 1.0 - uv.y;
    vec2 cell = vec2(1.0 / float(cols), 1.0 / float(rows));
    vec2 uv_fb = uv * cell + vec2(float(fx), float(fy)) * cell;
    vec4 tex = texture(u_Texture, uv_fb);
    color = tex;
    
}