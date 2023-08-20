#version 450

layout(location = 0) in vec3 forwards;

layout(set = 1, binding = 0) uniform samplerCube material;

layout(location = 0) out vec4 outColor;

void main() {
    float newY = forwards.y ;
// Define the range of y-values for the sky-like part
float skyMinY = -1.;  // Example: The lower limit of the sky
float skyMaxY = 1;  // Example: The upper limit of the sky
    newY = clamp(forwards.y, skyMinY, skyMaxY);
    float offset = 2.f;
    float newZ = clamp(-forwards.z,-1,0);
    newZ = -forwards.z;
    vec3 coords = vec3(forwards.x,newY,newZ);
	outColor = texture(material, coords);
	//outColor = vec4(0.0, 1.0, 0.0, 1.0);
}