#version 450

const vec2 quad[6] = vec2[]
(
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0)
);

layout(push_constant) uniform ScreenPC
{
    vec4 rect;      // x, y, w, h in pixels
    vec2 viewport;  // width, height in pixels
} pc;

layout(location = 0) out vec2 uv;

void main()
{
    vec2 local = quad[gl_VertexIndex];

    // local quad [0,1] -> pixel position
    vec2 pixelPos = pc.rect.xy + local * pc.rect.zw;

    // pixel -> NDC
    vec2 ndc;
    ndc.x = 2.0 * pixelPos.x / pc.viewport.x - 1.0;
    ndc.y = 2.0 * pixelPos.y / pc.viewport.y - 1.0;

    // flip Y if your screen-space origin is top-left
    ndc.y = -ndc.y;

    gl_Position = vec4(ndc, 0.0, 1.0);
    uv = local;
}