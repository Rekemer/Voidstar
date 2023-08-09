#version 450

layout(location = 0) in vec3 forwards;

layout(set = 1, binding = 0) uniform samplerCube material;

layout(location = 0) out vec4 outColor;

void main() {
    float newY = forwards.y ;
    vec3 coords = vec3(-forwards.x,newY,forwards.z);
	outColor = texture(material, coords);
	//outColor = vec4(0.0, 1.0, 0.0, 1.0);
}