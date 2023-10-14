#include "Prereq.h"
#include "Generation.h"
#include "glm.hpp"
namespace Voidstar
{

#define YisUP 1;
QuadData GeneratePlane(float detail)
{
		std::vector<Vertex> vertices = {};
		std::vector<IndexType> indices = {};
		int numDivisions = static_cast<int>(detail);
		float stepSize = 1.0f / numDivisions;
	
		for (int i = 0; i <= numDivisions; ++i)
		{
			for (int j = 0; j <= numDivisions; ++j)
			{
				Vertex vertex;
	
				// Calculate vertex position
				vertex.Position.x = i * stepSize - 0.5f;
				#if YisUP
				vertex.Position.y = j * stepSize - 0.5f;
				vertex.Position.z = 0;
				#else
				vertex.Position.y = 0;
				vertex.Position.z = j * stepSize - 0.5f;
				
				#endif // YisUP
	
				//// Calculate vertex normal
				//vertex.nx = 0.0f;
				//vertex.ny = 1.0f;
				//vertex.nz = 0.0f;
	
				// Calculate texture coordinates
				vertex.UV.x = static_cast<float>(i) / numDivisions;
				vertex.UV.y = static_cast<float>(j) / numDivisions;
	
				// Add the vertex to the vector
				vertices.push_back(vertex);
			}
		}
		// Generate indices for the plane
		for (int i = 0; i < numDivisions; ++i)
		{
			for (int j = 0; j < numDivisions; ++j)
			{
				int amountOfRowVerticies = (numDivisions + 1);
				// Calculate indices for the current quad
				unsigned int topLeft = i * amountOfRowVerticies + j;
				unsigned int topRight = topLeft + 1;
				unsigned int bottomLeft = topLeft + amountOfRowVerticies;
				unsigned int bottomRight = bottomLeft + 1;
				//unsigned int bottomRight = topRight + amountOfRowVerticies-1;
				//unsigned int bottomLeft = topLeft + amountOfRowVerticies+1;
	#define TRIANGLE 1
	#if TRIANGLE
		// Add the indices to the vector
				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
				indices.push_back(topRight);
	
				indices.push_back(topRight);
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
	#else
	// patch
				indices.push_back(topLeft);
				indices.push_back(topRight);
				indices.push_back(bottomRight);
				indices.push_back(bottomLeft);
	#endif
	
			}
		}
	
		return {vertices,indices};
	}

std::vector<Vertex> GenerateSphere(float radius, int rings, int sectors, std::vector<IndexType>& indices)
	{
		std::vector<Vertex> vertices;
		float const R = 1.0f / static_cast<float>(rings - 1);
		float const S = 1.0f / static_cast<float>(sectors - 1);
		{
	
			int r, s;
			float pi = 3.14159;
			for (r = 0; r < rings; ++r) {
				for (s = 0; s < sectors; ++s) {
					float const y = glm::sin(-pi * 2 + pi * r * R);
					float const x = glm::cos(2 * pi * s * S) * glm::sin(pi * r * R);
					float const z = glm::sin(2 * pi * s * S) * glm::sin(pi * r * R);
	
					Vertex vertex;
					vertex.Position.x  = x * radius;
					vertex.Position.y  = y * radius;
					vertex.Position.z  = z * radius;
	
					vertices.push_back(vertex);
				}
			}
		}
	
		{
			int r, s;
	
			for (r = 0; r < rings - 1; ++r) {
				for (s = 0; s < sectors - 1; ++s) {
					int first = r * sectors + s;
					int second = (r + 1) * sectors + s;
					int third = (r + 1) * sectors + (s + 1);
					int fourth = r * sectors + (s + 1);
	
					indices.push_back(first);
					indices.push_back(second);
					indices.push_back(third);
	
					indices.push_back(first);
					indices.push_back(third);
					indices.push_back(fourth);
				}
			}
		}
	
	
	
		return vertices;
	}
}
