#pragma once
#include <vector>
#include "Vertex.h"
#include "IndexBuffer.h"
namespace Voidstar
{
	struct QuadData
	{
		std::vector<Vertex> verticies;
		std::vector<IndexType> indicies;
	};

	QuadData GeneratePlane(float detail);
	std::vector<Vertex> GenerateSphere(float radius, int rings, int sectors, std::vector<IndexType>& indices);
}