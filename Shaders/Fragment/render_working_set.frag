#version 450





layout(set = 1, binding = 0) uniform sampler2DArray WorkingSet;
layout(set = 1, binding = 1) uniform sampler2D  PageTable;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec4 outColor;
vec2 pageSize = vec2(128,64);
float borderWidth = 2 ;
vec2 pageSizeMod = pageSize - borderWidth;

#define debug 0

void main ()
{
    vec2 workingSetSize =  vec2(1024,512);
    vec2 rescale =  pageSize/workingSetSize;
    vec4 index = texture(PageTable,uv);
    float amountOfTiles =exp2(index.b);
    vec2 offset = (fract(uv * amountOfTiles))  ;
    

    vec2 workingSetUv = offset;

    int layer  = int(index.y * 10+ index.x); 
    vec4 tex = texture(WorkingSet,vec3(workingSetUv ,layer)) ;
    
    
    outColor.xyzw =tex.xyzw;
    //outColor = vec4(index.xy,0,1);
   // outColor = vec4(workingSetUv,0,1); 
    
    
    
    #if debug
    
   if (amountOfTiles == 0)
   {
       outColor = vec4(1,0,1,1);
   }
   else if (amountOfTiles == 2)
   {
       outColor = vec4(1,0,0,1);
   }
   else if (amountOfTiles == 4)
   {
       outColor = vec4(0,0,1,1);
   }
   else if (amountOfTiles == 8)
   {
       outColor = vec4(1,0,0,1);
   }
   else if (amountOfTiles == 16)
   {
       outColor = vec4(1,1,1,1);
   }
   else if (amountOfTiles >16)
   {
       outColor = vec4(0,0,0,1);
   }
   #endif
    //outColor.xyzw =vec4(1,1,0,1);
    outColor.a =1;
}