
#version 450
layout(location = 0) out vec4 o_color;
layout(location = 0) in vec2 vUV;

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
} ubo;

float distance_from_sphere(in vec3 p, in vec3 c, float r)
{
    return length(p - c) - r;
}
const vec3  spherePos = vec3(0.0,0,4);
vec4 ray_march(in vec3 ro, in vec3 rd)
{
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 32;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = 1000.0;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;

        float distance_to_closest = distance_from_sphere(current_position, 
      spherePos, 1.0);

        if (distance_to_closest < MINIMUM_HIT_DISTANCE) 
        {
            return vec4(1.0, 0.0, 0.0,1.0);
        }

        if (total_distance_traveled > MAXIMUM_TRACE_DISTANCE)
        {
            break;
        }
        total_distance_traveled += distance_to_closest;
    }
    return vec4(0.0);
}

void main()
{   
    const int res = 100;
	vec2 aspect = vec2( 16 * res, 9 * res);
    vec2 uv = vUV.st * 2.0 - 1.0;
    //uv.x*= aspect.x/aspect.y;
    vec3 camera_position = vec3(0.0, 0.0, -15.0);
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

    o_color = vec4(shaded_color);
   // o_color = vec4(uv,0,1);
}