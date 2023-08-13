#version 450



const vec2 screen_corners[6] = vec2[](
	vec2(-1.0, -1.0), // left top
	vec2(-1.0,  1.0), // left bottom
	vec2( 1.0,  1.0), // right bottom
	vec2( 1.0,  1.0), // right bottom
	vec2( 1.0, -1.0), // right top
	vec2(-1.0, -1.0)  // left top
);



layout(location = 0) out vec2 uv;

void main() 
{	
	vec2 pos = screen_corners[gl_VertexIndex];
    uv = (pos + 1.0) * 0.5;
	gl_Position = vec4(pos, 0.0, 1.0);
	



}