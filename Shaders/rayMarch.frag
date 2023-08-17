
#version 450
layout(location = 0) out vec4 o_color;
layout(location = 0) in vec2 vUV;

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
    float time;
} ubo;



//layout(set = 1, binding = 0) uniform sampler2D[2]  u_Tex;
layout(set = 1, binding = 0) uniform sampler3D u_worleyNoise;
layout(set=1,binding = 1) uniform CloudParams {
    float densityOffset;
	float densityMult;
    vec4 weights;
} cloudParams;

float sdfBox(vec3 p, vec3 position, vec3 size)
{
    vec3 d = abs(p - position) - size;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

float distance_from_sphere(in vec3 p, in vec3 c, float r)
{
    return length(p - c) - r;
}
vec4 ray_march(in vec3 ro, in vec3 rd)
{
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 64;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = 1000.0;
    vec4 density = vec4(0,0,0,1);
    float absorbption = 1.1f;
     float minEntry = 0.f;
     float maxExit  = 0.f;
     float scale = 10;
    const vec3  spherePos = vec3(0,120,4);

      vec3 invRd = 1.0 / rd;
       vec3 cubeMin = vec3(-scale,-scale/5,-scale)+spherePos;
       vec3 cubeMax =  vec3(scale,scale/5,scale)+spherePos;
       vec3 t1 = (cubeMin - ro) * invRd;
       vec3 t2 = (cubeMax - ro) * invRd;
       vec3 tmin = min(t1, t2);
       vec3 tmax = max(t1, t2);
       minEntry = max(max(tmin.x, tmin.y), tmin.z);
       maxExit = min(min(tmax.x, tmax.y), tmax.z);
       float distanceInsideCube = max(0.0, maxExit - minEntry);
       float segmentsDistance = distanceInsideCube/NUMBER_OF_STEPS;
       distanceInsideCube = 0;
       vec3 current_position = ro;
       float totalDensity = 0;
       vec3 color = vec3(0,0,0);
    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
         current_position = ro + total_distance_traveled * rd;

        //float distance_to_closest = distance_from_sphere(current_position, spherePos, 1.0);
        //scale *=(1- i/NUMBER_OF_STEPS);
        float distance_to_closest = sdfBox(current_position,spherePos, vec3(scale,scale/5,scale));
       //distance_to_closest = 0.1f;
     
      
       
     
     
       
       // Calculate the distance between entry and exit points
      



       if (distance_to_closest < MINIMUM_HIT_DISTANCE) 
       {
         
          for (int j = 0; j < NUMBER_OF_STEPS; ++j)
        {
             current_position = ro + total_distance_traveled * rd;
            vec3 texCoords= (current_position.xyz - spherePos);
            texCoords.xz/=scale;
            texCoords.y/=scale;
            texCoords=(texCoords+1)/2;
            color = texture(u_worleyNoise,fract(texCoords)).xxx;
            float densityOffset = cloudParams.densityOffset;
            float densityMult =cloudParams.densityMult;
            float baseShapeDensity = color.x + densityOffset * .1;

            float densityFromTex =color.x ;
         color.x = baseShapeDensity;
        // break;
         //color.x = pow(color.x,densityMult); 
         color.x *=densityMult; 
         totalDensity +=densityFromTex*segmentsDistance;
          total_distance_traveled+=segmentsDistance;
        }   
       break;
         //color = mix(vec3(0,0,0), color, 0.6);
        // break;
         //color = texCoords;
        // break;
         //texCoords.x /=14;
        // texCoords.y /=2;
           //density.x+=0.1;
           //distanceInsideCube+=segmentsDistance*densityFromTex;

          // return vec4(1.0, 0.0, 0.0,1.0);
       }
       
       if (total_distance_traveled > MAXIMUM_TRACE_DISTANCE)
       {
          return vec4(0.4,0.4,0.4,0);
          break;
       }
        total_distance_traveled += distance_to_closest;
    }
    density.x = exp(-totalDensity);
    density.w = exp(-totalDensity);
    vec3 pinkColor = vec3(100.f/255.f,71.f/255.f,76.f/255.f);
    //return vec4(color,1);
    return vec4(pinkColor*density.xxx,density.w);
}

void main()
{   
    const int res = 100;
	vec2 aspect = vec2( 16 * res, 9 * res);
    vec2 uv = vUV.st * 2.0 - 1.0;
    //uv.x*= aspect.x/aspect.y;
    vec3 camera_position = vec3(0.0, 0.0, -15.0);
    camera_position = vec3(inverse(ubo.view)[3]);
    vec3 ro = camera_position;
  //  vec3 direction =  normalize(spherePos - ro); 
    vec3 rd = vec3(uv, 1.0);

    //world space
    vec4 rd_camera = inverse(ubo.proj)*vec4(rd,1);
    rd_camera/= rd_camera.w;
    rd_camera = normalize(rd_camera);
    vec4 rd_world = inverse(ubo.view) * rd_camera;

   //vec3 rd =direction;

    vec4 shaded_color = ray_march(ro, rd_world.xyz);

    o_color = vec4(shaded_color.xxx,shaded_color.w);
   // o_color = vec4(uv,0,1);
}