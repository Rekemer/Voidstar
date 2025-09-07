#pragma once
#include <vector>
#include "Vertex_.h"
#include "IndexBuffer.h"
namespace Voidstar
{
	struct  QuadData
	{
		std::vector<Vertex_> verticies;
		std::vector<IndexType> indicies;
	};

	QuadData GeneratePlane(float detail);
	std::vector<Vertex_> GenerateSphere(float radius, float prec, std::vector<IndexType>& indices);

	std::vector<Vertex_> GetCube();
	

}