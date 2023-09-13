#version 450



layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;


layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
    float time;
    
} ubo;

layout(set = 1, binding = 0) uniform sampler2D u_Selected;
layout(set = 1, binding = 1) uniform sampler2D u_Noise1;

float scale = 100;




float lerp(float a, float b, float t)
{
   
    return min(a,b) + t*(max(a,b)-a);
}




vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 meatBalls(vec2 st)
{
     // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);

    float m_dist = 1.;  // minimum distance
    for (int j= -1; j <= 1; j++ ) {
        for (int i= -1; i <= 1; i++ ) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(i),float(j));

            // Random position from current + neighbor place in the grid
            vec2 point = random2(i_st + neighbor);

            // Animate the offset
           // offset = 0.5 + 0.5*sin(u_time + 6.2831*offset);

            // Position of the cell
            vec2 pos = neighbor + point - f_st;

            // Cell distance
            float dist = length(pos);

            // Metaball it!
            m_dist = min(m_dist, m_dist*dist);
        }
    }
    return vec3(step(0.160, m_dist));
}

float catRom(float x, float a,float a1,float a2,float a3)
{
    return pow(x,3) * a3 +pow(x,2) * a2 +pow(x,1) * a1 +a;
}

vec4 sampleNeighboringTexel(sampler2D textureSampler, vec2 uv,vec2 offset)
{
    // Define the texel size, assuming normalized UV coordinates
   vec2 texelSize = 1.0 / textureSize(textureSampler, 0);
   vec2 neighborUV = uv + offset * texelSize;
   vec4 neighborColors = texture(textureSampler, neighborUV);


    return neighborColors;
}

float bicubic(vec2 scaledUv)
{
    vec2 indexUv = floor(scaledUv);
   
    vec2 f = fract(scaledUv);
   // f = vec2(1)-f;
    float splineX[4][4];

    for (int i =0; i< 4; i++)
    {
        for (int j =0; j< 4; j++)
        {
            
            splineX[i][j] = texture(u_Selected,uv + vec2(i-1,j-1)/(scale/6)).x;
        }
    }
    
    float splineY[4];
    for (int i=0; i <4; i++)
    {
        float ya = splineX[0][i];
        float yb = splineX[1][i];
        float yc = splineX[2][i];
        float yd = splineX[3][i];
        float a0 = yb;
        float a1 = 0.5 *(-ya+yc);
        float a2 = 0.5 * (2*ya-5*yb+4*yc -yd);   
        float a3 = 0.5 * (-ya+3*yb-3*yc +yd);   
        splineY[i] = catRom(f.x,a0,a1,a2,a3);
    }

    float ya = splineY[0];
    float yb = splineY[1];
    float yc = splineY[2];
    float yd = splineY[3];
    float a0 = yb;
    float a1 = 0.5 *(-ya+yc);
    float a2 = 0.5 * (2*ya-5*yb+4*yc -yd);   
    float a3 = 0.5 * (-ya+3*yb-3*yc +yd);   
    float res = catRom(f.y,a0,a1,a2,a3);
    return clamp(res,0,1);

}



float gradientNoise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

void main() 
{


    
    const vec2 quadNumber= vec2(scale/2,scale/2);
    vec2 scaledUv = uv*scale;
    vec2 indexUv = floor(scaledUv);
    vec2 fractedUv = fract(scaledUv );

    vec2 offset = random2(indexUv)*vec2(10.0, 0.0); // Offset one texel to the right
    //vec4 selectedShape = sampleNeighboringTexel(u_Selected,uv,offset);
    vec2 rand  = random2(uv);
    vec4 noise_1 =1-texture(u_Noise1,uv*.0025); 
    vec4 selectedShape =texture(u_Selected,uv); 
  
  
//float maxLen = length(vec2(100,100));
  //  float len = length(((quadNumber)-(indexUv)))/(maxLen*1.);
    float indexLen = length(((quadNumber)-(indexUv)));


    
   

    
   vec3 balls = meatBalls(scaledUv/20);
   // outColor.xyz = balls;
   // outColor.xyz = balls;
   // outColor.a = 1;
 
     // Tile the space
    vec2 i_st = indexUv;
    vec2 f_st = fractedUv;

    vec3 m_dist = vec3(1);  // minimum distance
     const float circleAmount = 120.f;
    for (int j= -1; j <=1; j++ ) {

        for (int i= -1; i <= 1; i++ ) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(i),float(j));
                // Random position from current + neighbor place in the grid
                vec2 point = random2(i_st + neighbor);
                // Animate the offset
                // offset = 0.5 + 0.5*sin(u_time + 6.2831*offset);
                point =  0.5 + 0.5*sin(ubo.time*1000 + 6.2831*point);
                // Position of the cell
                vec2 pos = neighbor + point - f_st;

                // Cell distance
                float dist = length(pos);

                // Metaball it!
                //m_dist = min(m_dist, m_dist*dist)*(1-selectedShape.x)*(1-noise_1.x);
                m_dist = min(m_dist, m_dist*dist)* (1 - selectedShape.xyz) ;
            
        }
    }   
    vec4 shape = selectedShape;
    float randomDispl =gradientNoise(uv*20); 

      outColor = shape;
     //outColor = vec4(randomDispl ,randomDispl ,randomDispl ,1);
      //m_dist = m_dist+
      vec4 fillColor = vec4(0,0,1,1);
      vec3 res = step(vec3(0.0425),m_dist);
     // res = bicubic(scaledUv);
      outColor =vec4(vec3(res),1);
     // outColor =vec4(uv,0,1);
      //outColor = fillColor*(1-selectedShape);
      //outColor= noise_1;
      outColor.a = 1;
    outColor.xyz = 1- selectedShape.xyz;


}