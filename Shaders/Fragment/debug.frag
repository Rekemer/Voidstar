#version 450
layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;
layout(location = 0) in vec2 uv ;
layout(location = 0) out vec4 color ;
layout(set = 1, binding = 0) uniform sampler2DArray u_Tex;
layout(set=1,   binding = 1) uniform AddData {
    bool debug;
} addData;

void main() 
{   
    
    vec2 pageSize = vec2(128,64);
    vec2 workingSetSize =  vec2(1280,640);
    vec2 rescale =  pageSize/workingSetSize;

    vec2 grid = fract(uv*20)*rescale * 10;
    vec2 index = floor(uv*20);
    int layer = int(index.y * 20.f + index.x);
    color = texture(u_Tex,vec3(grid,layer));
    if (!addData.debug)
    {
        color.a = 0;
    }
    //color = vec4(grid,0,1);
    //color = vec4(addData.debug,addData.debug,addData.debug,1);
}