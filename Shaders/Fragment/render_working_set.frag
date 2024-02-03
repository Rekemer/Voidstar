#version 450





#define mipMaps 4
layout(set = 1, binding = 0) uniform sampler2D WorkingSet;
layout(set = 1, binding = 1) uniform sampler2D  PageTable;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec4 outColor;
vec2 pageSize = vec2(128,64);
vec2 megatextureSize = vec2(1024,512);

#define debug 0

void main ()
{
    vec2 workingSetSize =  textureSize(WorkingSet, 0);
    vec4 index = texture(PageTable,uv);
    float amountOfTiles =exp2(index.b);
    vec2 rescale =  pageSize/workingSetSize;
    vec2 offset = fract(uv * amountOfTiles) *rescale;

    vec4 tex = texture(WorkingSet,(index.xy+offset )) ;
    
    
    outColor.xyzw =tex.xyzw;
    //outColor = vec4(index.xy,0,1);
    //outColor = vec4(uv,0,1); 
    
    
    
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
    //outColor.xyzw =vec4(offset,0,1);
    outColor.a =1;
}