#version 450

layout(location = 0) out vec4  outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;


#define maxMipLevel 3
float
mip_map_level(in vec2 texture_coordinate)
{
    // The OpenGL Graphics System: A Specification 4.2
    //  - chapter 3.9.11, equation 3.21


    vec2  dx_vtc        = dFdx(texture_coordinate);
    vec2  dy_vtc        = dFdy(texture_coordinate);
    float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));

    //return max(0.0, 0.5 * log2(delta_max_sqr) - 1.0); // == log2(sqrt(delta_max_sqr));
    float miplevel = 0.5 * log2(delta_max_sqr);// == log2(sqrt(delta_max_sqr));
    miplevel = clamp(miplevel,0,maxMipLevel); 
    miplevel = maxMipLevel  - miplevel;
    return miplevel; 
}

float tex_mip_level(vec2 coord, vec2 tex_size)
{
   vec2 dx_scaled, dy_scaled;
   vec2 coord_scaled = coord * tex_size;

   dx_scaled = dFdx(coord_scaled);
   dy_scaled = dFdy(coord_scaled);
   float d = max(dot(dx_scaled, dx_scaled),dot(dy_scaled, dy_scaled));
   float miplevel = log2(sqrt(d)); 
   miplevel = clamp(miplevel,0,maxMipLevel); 
   miplevel = maxMipLevel  - miplevel;
   return miplevel;
}

vec2 pageSize = vec2(128,64);
vec2 megatextureSize = vec2(1024,512);
vec2 downscale = vec2(28,16) / vec2(1440,810);
void main() 
{
    //float mipLevel =  mip_map_level(uv * megatextureSize);
    float mipLevel =  tex_mip_level(uv , megatextureSize* downscale);
    mipLevel = floor(mipLevel);
    float amountOfTiles = exp2(mipLevel);
    vec2 pageNumber = amountOfTiles * uv;
    pageNumber.x = clamp(pageNumber.x,0,exp2(mipLevel)-1);
    pageNumber.y = clamp(pageNumber.y,0,exp2(mipLevel)-1);
    outColor = vec4(floor(pageNumber),mipLevel,1);
}