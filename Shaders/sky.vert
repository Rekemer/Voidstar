#version 450

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
} ubo;

layout(location = 0) out vec3 forwards;

const vec2 screen_corners[6] = vec2[](
	vec2(-1.0, -1.0), // left top
	vec2(-1.0,  1.0), // left bottom
	vec2( 1.0,  1.0), // right bottom
	vec2( 1.0,  1.0), // right bottom
	vec2( 1.0, -1.0), // right top
	vec2(-1.0, -1.0)  // left top
);

void main() {
    // Extract the forward, up, and right vectors from the view matrix
    vec3 fragForward = normalize(ubo.view[2].xyz);
    vec3 fragUp = normalize(ubo.view[1].xyz);
    vec3 fragRight = normalize(ubo.view[0].xyz);
  
    //fragForward = vec3 (0.7f,  0.3f, 0.0f);
	//fragRight =    vec3(   0.0f, -1.0f, 0.0f);
	//fragUp =       vec3( 0.0f,  0.0f, 1.0f);
	vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.0, 1.0);
	forwards = normalize(fragForward + pos.x * fragRight + pos.y * fragUp).xyz;
}