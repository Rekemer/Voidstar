#pragma once
#include <vector>
#include "Vertex.h"
#include "IndexBuffer.h"
namespace Voidstar
{
	std::vector<Vertex> GeneratePlane(float detail, std::vector<IndexType>& indices);
	std::vector<Vertex> GenerateSphere(float radius, int rings, int sectors, std::vector<IndexType>& indices);
}