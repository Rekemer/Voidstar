#version 450

layout(set=0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 playerPos;
} ubo;

layout(location = 0) out vec3 forwards;

const vec2 screen_corners[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0),
	vec2( 1.0,  1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0, -1.0)
);

void main() {
    // Extract the forward, up, and right vectors from the view matrix
    vec3 fragForward = normalize(-ubo.view[2].xyz);
    vec3 fragUp = normalize(ubo.view[1].xyz);
    vec3 fragRight = normalize(ubo.view[0].xyz);
	vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.0, 1.0);
	forwards = normalize(fragForward + pos.x * fragRight - pos.y * fragUp).xyz;
}