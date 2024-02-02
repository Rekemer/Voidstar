#version 450


float tex_mip_level(vec2 coord, vec2 tex_size)
{
   vec2 dx_scaled, dy_scaled;
   vec2 coord_scaled = coord * tex_size;

   dx_scaled = dFdx(coord_scaled);
   dy_scaled = dFdy(coord_scaled);

   vec2 dtex = dx_scaled*dx_scaled + dy_scaled*dy_scaled;
   float min_delta = max(dtex.x,dtex.y);
   float miplevel = max(0.5 * log2(min_delta), 0.0);

   return miplevel;
}



#define mipMaps 4
layout(set = 1, binding = 0) uniform sampler2D WorkingSet;
layout(set = 1, binding = 1) uniform sampler2D  PageTable[mipMaps];
layout(location = 1) in vec2 uv;
layout(location = 0) out vec4 outColor;
vec2 pageSize = vec2(128,64);
vec2 megatextureSize = vec2(1024,512);

#define debug 0

void main ()
{
    int mipLevel =  int(clamp(floor(tex_mip_level(uv,megatextureSize)),0,3));
    vec2 workingSetSize =  textureSize(WorkingSet, 0);
    mipLevel = 0;
    // we missing at level 0 with current uv 
    vec4 index = texture(PageTable[int(mod(3-mipLevel,3))],uv) ;
    index = vec4(0,0,0,0);
    ivec2 i =ivec2(index.xy* workingSetSize);
    int amountOfTiles = int(exp2(mod(3-mipLevel,3)));
   // amountOfTiles = 1;
    vec2 pageSizeWorkingSet = workingSetSize/pageSize;

    vec2 f = fract (uv * amountOfTiles) * 1/pageSizeWorkingSet ;
    vec4 tex = texture(WorkingSet,(index.xy+f));
    outColor.xyzw =tex.xyzw;
    //outColor = vec4(index.xy,0,1);
    
    
    
    
    #if debug
    
   if (amountOfTiles == 0)
   {
       outColor = vec4(1,0,1,1);
   }
   else if (amountOfTiles == 1)
   {
       outColor = vec4(1,0,0,1);
   }
   else if (amountOfTiles == 2)
   {
       outColor = vec4(0,0,1,1);
   }
   else if (amountOfTiles == 3)
   {
       outColor = vec4(1,0,0,1);
   }
   else if (amountOfTiles >4)
   {
       outColor = vec4(0,0,0,1);
   }
   #endif
    //outColor.xyzw =vec4(f,0,1);
    //outColor.a =1;
}