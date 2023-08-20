
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

    vec3 lightDir;
    vec3 lightPos;
    vec3 cloudPos;
    vec3 cloudBoxScale;

    float lightAbsorption;
    float aHg;


} cloudParams;
layout(set = 1, binding = 2) uniform sampler3D u_worleyNoiseLowRes;

float sdfBox(vec3 p, vec3 position, vec3 size)
{
    vec3 d = abs(p - position) - size;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

float distance_from_sphere(in vec3 p, in vec3 c, float r)
{
    return length(p - c) - r;
}


vec2 GetMinMaxBox(vec3 boxScale, vec3 boxPos, vec3 ro,vec3 rd)
{
    float minEntry = 0.f;
     float maxExit  = 0.f;
  
      vec3 invRd = 1.0 / rd;
    
       vec3 cubeMin = vec3(-boxScale)+boxPos;
       vec3 cubeMax =  vec3(boxScale)+boxPos;
       vec3 t1 = (cubeMin - ro) * invRd;
       vec3 t2 = (cubeMax - ro) * invRd;
       vec3 tmin = min(t1, t2);
       vec3 tmax = max(t1, t2);
       minEntry = max(max(tmin.x, tmin.y), tmin.z);
       maxExit = min(min(tmax.x, tmax.y), tmax.z);
       return vec2(minEntry,maxExit);
}

// Henyey-Greenstein
float hg(float a, float g) {
    float g2 = g*g;
    return (1-g2) / (4*3.1415*pow(1+g2-2*g*(a), 1.5));
}


float sampleDenstiy( vec3 rayPos)
{   
    float totalDensity = 0;
    vec3 cloudPos = cloudParams.cloudPos;
    vec3 cloudBoxScale= cloudParams.cloudBoxScale;
    vec3 texCoords= (rayPos.xyz - cloudPos);
    texCoords/=cloudBoxScale;
    texCoords=(texCoords+1)/2;
    vec3 detailedTex = texture(u_worleyNoise,fract(texCoords)).xxx;
    vec3 densityFromLowerResTex = texture(u_worleyNoiseLowRes,fract(texCoords)).xxx;
    float densityOffset = cloudParams.densityOffset;
    float densityMult =cloudParams.densityMult;
    float baseShape = densityFromLowerResTex.x + densityOffset * .01;
    float detailedShape = detailedTex.x;
   // float densityFromTex =detailedShape*densityMult;
    //detailedTex.xyz = densityFromLowerResTex.xxx;
    //break;
    totalDensity =baseShape.x ;
    if(totalDensity >0)
    {
        float oneMinusShape = 1 ;
        float detailErodeWeight = oneMinusShape *  oneMinusShape* oneMinusShape;
       // totalDensity*=totalDensity*detailErodeWeight;
        totalDensity+=totalDensity- (1-detailedShape.x) * detailErodeWeight;
        //totalDensity+= densityOffset * .01f;
    }
    return totalDensity;
     // Subtract detail noise from base shape (weighted by inverse density so that edges get eroded more than centre)
   //float oneMinusShape = 1 - densityFromLowerResTex.x;
   //float detailErodeWeight = oneMinusShape * oneMinusShape * oneMinusShape;
   //float cloudDensity = detailedShape - (1-detailedTex.x) * detailErodeWeight;

   //densityFromTex+=densityFromLowerResTex.x;
   //return densityFromTex;
   //return cloudDensity;
}

vec4 ray_march(in vec3 ro, in vec3 rd)
{
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 64;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    vec4 density = vec4(0,0,0,1);
    float absorbption = 1.1f;
     float minEntry = 0.f;
     float maxExit  = 0.f;
  
      vec3 cloudPos = cloudParams.cloudPos;
      vec3 cloudBoxScale= cloudParams.cloudBoxScale;
      vec2 minMax = GetMinMaxBox(cloudBoxScale, cloudPos, ro, rd);
      minEntry = minMax.x;
      maxExit = minMax.y;
       float distanceInsideCube = max(0.0, maxExit - minEntry);
       float segmentsDistance = distanceInsideCube/NUMBER_OF_STEPS;
       distanceInsideCube = 0;

       vec3 current_position = ro+sdfBox(ro,cloudPos, vec3(cloudBoxScale))*rd;
       float totalDensity = 0;
       vec3 color = vec3(0,0,0);
       float totalDensityLight = 0;
    const float MAXIMUM_TRACE_DISTANCE = 100000.0;
     vec3 lightDir = normalize(cloudParams.lightDir);
    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
         current_position = ro + total_distance_traveled * rd;

        //float distance_to_closest = distance_from_sphere(current_position, spherePos, 1.0);
        //scale *=(1- i/NUMBER_OF_STEPS);
        float distance_to_closest = sdfBox(current_position,cloudPos, vec3(cloudBoxScale));
       //distance_to_closest = 0.1f;
     
      
       
     
     
       
       // Calculate the distance between entry and exit points
      



       
       if (distance_to_closest < MINIMUM_HIT_DISTANCE) 
       {
         
          for (int j = 0; j < NUMBER_OF_STEPS; ++j)
          {
            current_position = ro + total_distance_traveled * rd;
            float densityFromTex = sampleDenstiy(current_position);
          
            totalDensity +=max(0,densityFromTex*segmentsDistance);
            total_distance_traveled+=segmentsDistance;
            vec2 minMax = GetMinMaxBox(cloudBoxScale, cloudPos, current_position, -lightDir);

            float distanceInsideCubeLight = max(0.0, minMax.y);
            if( minMax.y<0 && minMax.x<0)
            {
                //    return vec4(1,0,1,1);
            }
            //if(minMax.y > 10)
            
            const float NUMBER_OF_STEPS_LIGHT =32;
            
            float segmentsDistanceLight = distanceInsideCubeLight/NUMBER_OF_STEPS_LIGHT;
            if( segmentsDistanceLight<=0)
            {
            //        return vec4(1,1,0,1);
            }
            vec3 lightPos = current_position+ segmentsDistanceLight* -lightDir;
            for (int jj = 0; jj < NUMBER_OF_STEPS_LIGHT; ++jj)
            {
             //vec3 texCoords= (lightPos.xyz - cloudPos);
             //texCoords/=cloudBoxScale;
             //texCoords=(texCoords+1)/2;
             //float density = texture(u_worleyNoise,fract(texCoords)).x;
             //float densityFromLowerResTex = texture(u_worleyNoiseLowRes,fract(texCoords)).x;
              float densityFromTex = sampleDenstiy(current_position);
              float angle = dot(rd,-lightDir);
              float a =cloudParams.aHg;
              totalDensityLight +=max(0,(densityFromTex)*segmentsDistanceLight)*hg(angle,a);
              lightPos = lightPos+ segmentsDistanceLight* -lightDir;
            }
          }   
       break;
       }
       
       if (total_distance_traveled > MAXIMUM_TRACE_DISTANCE)
       {
          return vec4(0.4,0.4,0.4,0);
          break;
       }
        total_distance_traveled += distance_to_closest;
    }
   // float transmittance = exp(-totalDensityLight*3);
  //  float angle = dot(rd,-lightDir);
   // float a =cloudParams.aHg;
    density.x = totalDensity;
    density.w = totalDensity;
    float lightTransmittance = exp(-totalDensityLight*cloudParams.lightAbsorption)  ;
    
    //return vec4(totalDensity,totalDensity,totalDensity,1);
    //return vec4(color,1);
    //totalDensity = 1-totalDensity;
    float res = 1 - totalDensity*lightTransmittance;
    vec4 finalColor = vec4(vec3(res),res);
    return finalColor;
}
const float far = 10000;
 const float near = 10.00;
float linerizeDepth(float depth)
{
    float ndc = depth * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
    linearDepth/=far;
    return linearDepth;
}
layout (input_attachment_index = 0, set = 0, binding = 1) uniform subpassInputMS inputDepth;
void main()
{   

        float depthValue = subpassLoad(inputDepth,2).r;
        float linearCurrentDepth = linerizeDepth(gl_FragCoord.z);
        if (depthValue < linearCurrentDepth)
        {
            discard;
        }

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
         vec3 pinkColor = vec3(100.f/255.f,71.f/255.f,76.f/255.f);
      //  o_color = vec4(shaded_color.x*pinkColor,shaded_color.w);
     o_color = vec4((shaded_color.xyz),shaded_color.w);
   //o_color = vec4(uv,0,1);
}