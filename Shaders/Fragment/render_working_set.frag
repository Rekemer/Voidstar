
layout(set = 1, binding = 0) uniform sampler2D WorkingSet;
layout(set = 1, binding = 1) uniform sampler2D PageTable;

in vec2 uv;
out vec4 outColor;

void main{
    outColor = vec4(1,0,1,1);
}