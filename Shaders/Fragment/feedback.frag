#version 450

layout(location = 0) out vec4  outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;




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


const float pageWidth = 128;
const float pageHeight= 64;
void main() 
{
    outColor = color;  
    float miplevel = tex_mip_level(uv,vec2(128,64));
    //  log returns amount of tile on the mip level
    vec2 pageNumber = floor(pow(2,(8-miplevel)) * uv);
    outColor = vec4(pageNumber,miplevel,1);
    
   //outColor = vec4(0.2,0.3,3.4,1);
}