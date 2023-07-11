// tessellation control shader
#version 450


// specify number of control points per patch output
// this value controls the size of the input and output arrays
layout (vertices=4) out;

layout(location = 1) in vec4[] inColor ;
layout(location = 1) out vec4[] outColor ;
const int AB = 2;
const int BC = 3;
const int CD = 0;
const int DA = 1;

// Calculate tessellation levels
float lodFactor(float dist) 
{
    float u_tessellationFactor = 2;
    float u_tessellationSlope = 1;
    float u_tessellationShift = 0;
	float tessellationLevel = max(0.0, u_tessellationFactor/pow(dist, u_tessellationSlope) + u_tessellationShift);
	return tessellationLevel;
}
                //indices.push_back(topLeft);
				//indices.push_back(topRight);
				//indices.push_back(bottomRight);
				//indices.push_back(bottomLeft);

void main()
{
    vec4 targetPoint = vec4(0,0,0,0.);
    vec3 sumPosition = vec3(0.0);
    float maxLodLevel = 6;
    float minLodLevel = 1;
    int numPoints = 4;
	outColor[gl_InvocationID] = inColor[gl_InvocationID];
if(gl_InvocationID == 0)
{
    // ----------------------------------------------------------------------
    // Step 1: define constants to control tessellation parameters
	// set these as desired for your world scale
    const int MIN_TESS_LEVEL = 1;
    const int MAX_TESS_LEVEL = 1;
    const float MIN_DISTANCE = 0.01;
    const float MAX_DISTANCE = 10;

    // ----------------------------------------------------------------------
    // Step 2: transform each vertex into eye space
    vec4 eyeSpacePos00 =gl_in[0].gl_Position;
    vec4 eyeSpacePos01 =gl_in[1].gl_Position;
    vec4 eyeSpacePos10 =gl_in[2].gl_Position;
    vec4 eyeSpacePos11 =gl_in[3].gl_Position;

	float d00= length(eyeSpacePos00);
	float d01= length(eyeSpacePos01);
	float d10= length(eyeSpacePos10);
	float d11= length(eyeSpacePos11);

    // ----------------------------------------------------------------------
    // Step 3: "distance" from camera scaled between 0 and 1
    float distance00 = clamp((abs(d00)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
    float distance01 = clamp((abs(d01)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
    float distance10 = clamp((abs(d10)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
    float distance11 = clamp((abs(d11)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);

    // ----------------------------------------------------------------------
    // Step 4: interpolate edge tessellation level based on closer vertex
    float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00) );
    float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01) );
    float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11) );
    float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10) );


	float maxTileWitdth =25.f;
	float tileWidth =length(gl_in[0].gl_Position - gl_in[1].gl_Position);
	float fragments = maxTileWitdth/tileWidth;
    // ----------------------------------------------------------------------
    // Step 5: set the corresponding outer edge tessellation levels
    gl_TessLevelOuter[0] = tessLevel0;
    gl_TessLevelOuter[1] = tessLevel1;
    gl_TessLevelOuter[2] = tessLevel2;
    gl_TessLevelOuter[3] = tessLevel3;

    // ----------------------------------------------------------------------
    // Step 6: set the inner tessellation levels to the max of the two parallel edges
    gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
    gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
}
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

}
	