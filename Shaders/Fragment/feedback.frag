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




void main() 
{
    outColor = color;  
    outColor = vec4(uv,0,1);
}