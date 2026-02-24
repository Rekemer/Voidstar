#pragma once
#include "glm.hpp"
namespace Voidstar
{
	struct Vertex_
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 UV;
	};

	struct VertexModel_
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 UV;
		//glm::vec4 Tangent; 
	};
}
