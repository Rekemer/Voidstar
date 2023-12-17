#version 450



const vec2 screen_corners[6] = vec2[](
	vec2(-1.0, -1.0), // left top
	vec2(-1.0,  1.0), // left bottom
	vec2( 1.0,  1.0), // right bottom
	vec2( 1.0,  1.0), // right bottom
	vec2( 1.0, -1.0), // right top
	vec2(-1.0, -1.0)  // left top
);

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
} ubo;


// layout(location = 0) in vec3 in_pos;
// layout(location = 1) in vec2 in_uv;
 layout(location = 0) out vec2 uv;
// layout(location = 1) out vec3 view;
layout(set=1,binding = 1) uniform CloudParams {
    float densityOffset;
	float densityMult;
    vec4 weights;

    vec3 lightDir;
    vec3 lightPos;
    vec3 cloudPos;
    vec3 cloudBoxScale;


} cloudParams;

void main() 
{	
	vec2 pos = screen_corners[gl_VertexIndex];
    uv = (pos + 1.0) * 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
	//vec3 worldPos = (in_pos)*cloudParams.cloudBoxScale  + cloudParams.cloudPos; 
	//vec4 ndcSpace = (ubo.proj*ubo.view*vec4(worldPos,1));
	//gl_Position = ndcSpace;
	//pos = ndcSpace.xyz;
	//// float3 viewVector = mul(unity_CameraInvProjection, float4(v.uv * 2 - 1, 0, -1));
    ////output.viewVector = mul(unity_CameraToWorld, float4(viewVector,0));
	//vec4 viewVector = (inverse(ubo.proj) * vec4(in_uv*2-1,0,1));
	//view = (inverse(ubo.view) * viewVector).xyz;



}