// tessellation evaluation shader
#version 450


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    
} ubo;
layout(location = 1) in vec4[] inColor ;
layout(location = 1) out vec4 outColor ;
layout (quads) in;


void main()
{
   // Calculate the barycentric coordinates of the current point on the quad
    vec3 barycentricCoord = gl_TessCoord.xyz;
    // Interpolate the position within the quad using barycentric coordinates
    vec4 position = mix(
        mix(gl_in[0].gl_Position, gl_in[1].gl_Position, barycentricCoord.x),
        mix(gl_in[2].gl_Position, gl_in[3].gl_Position, barycentricCoord.x),
        barycentricCoord.z
    );
    // Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.z);


// get patch coordinate
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
// retrieve control point position coordinates
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

// bilinearly interpolate position coordinate across patch
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;
    
    gl_Position = ubo.proj*ubo.view*p;

     vec4 color = mix(
        mix(inColor[0], inColor[1], barycentricCoord.x),
        mix(inColor[2], inColor[3], barycentricCoord.x),
        barycentricCoord.z
    );
     outColor = color;
}