#pragma once
#include <vector>
#include "Vertex.h"
#include "IndexBuffer.h"
namespace Voidstar
{
	struct VOIDSTAR_API QuadData
	{
		std::vector<Vertex> verticies;
		std::vector<IndexType> indicies;
	};

	QuadData GeneratePlane(float detail);
	std::vector<Vertex> GenerateSphere(float radius, float prec, std::vector<IndexType>& indices);
}