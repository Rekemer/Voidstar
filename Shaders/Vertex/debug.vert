

#version 450

const float xSize = 1.2;
const float ySize = 1.2;
const vec2 screen_corners[6] = vec2[](
	vec2(-1.0, -1.0+ySize), // left top
	vec2(-1.0,  1.0), // left bottom
	vec2( 1.0 - xSize,  1.0), // right bottom
	vec2( 1.0 - xSize,  1.0), // right bottom
	vec2( 1.0 - xSize, -1.0 + ySize), // right top
	vec2(-1.0, -1.0+ySize)  // left top
);


const vec2 screen_corners_uv[6] = vec2[](
	vec2(0.0, 1.0), // left top
	vec2(0.0, 0.0), // left bottom
	vec2( 1.0,  0.0), // right bottom
	vec2( 1.0,  0.0), // right bottom
	vec2( 1.0, 1.0), // right top
	vec2(0.0, 1.0)  // left top
);


layout(location = 0) out vec2 uv;
void main() 
{
    
    uv = 1 - screen_corners_uv[gl_VertexIndex];
    vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.0, 1.0);

}