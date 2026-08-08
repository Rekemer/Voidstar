#pragma once
#pragma once
#include "glm.hpp"

// user defined verticies
struct Vertex
{
	glm::vec3 Position;
	//glm::vec4 Color;
	glm::vec2 UV;
	//alignas(4)
	//float textureID;
};
struct InstanceData
{
	glm::vec4 Color;
	alignas(16)glm::mat4 WorldMatrix;

};


