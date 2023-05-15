#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);


void main() 
{
	// mat4 world;
	// world[0]= vec4(1,0,0,0);
	// world[1]= vec4(0,1,0,0);
	// world[2]= vec4(0,0,1,0);
	// world[3]= vec4(0,0,-1,1);
	// world  = mat4
	// (1,0,0,0,
	//  0,1,0,0,	
	//  0,0,1,0,
	//  0,0,10,1
	// );
	//gl_Position = cameraData.viewProjection*cameraData.world*vec4(in_pos, 0, 1.0);
	gl_Position = vec4(positions[gl_VertexIndex],0,1);
	
	
}