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
float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }
std::vector<Vertex> GenerateSphere(float radius, float prec, std::vector<IndexType>& indices)
	{
		float numVertices = (prec + 1) * (prec + 1);
		float numIndices = prec * prec * 6;

		std::vector<Vertex> vertices;
		indices.resize(numIndices);
		vertices.resize(numVertices);
		for (int i = 0; i <= prec; i++) {
			for (int j = 0; j <= prec; j++) {
				float y = (float)cos(toRadians(180.0f - i * 180.0f / prec));
				float x = -(float)cos(toRadians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));
				float z = (float)sin(toRadians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));
				vertices[i * (prec + 1) + j].Position = glm::vec3(x, y, z);
				vertices[i * (prec + 1) + j].UV = glm::vec2(((float)j / prec), ((float)i / prec));
			}
		}
	
		// generate indexes
		for (int i = 0; i < prec; i++) {
			for (int j = 0; j < prec; j++) {
				indices[6 * (i * prec + j) + 0] = i * (prec + 1) + j;
				indices[6 * (i * prec + j) + 1] = i * (prec + 1) + j + 1;
				indices[6 * (i * prec + j) + 2] = (i + 1) * (prec + 1) + j;
				indices[6 * (i * prec + j) + 3] = i * (prec + 1) + j + 1;
				indices[6 * (i * prec + j) + 4] = (i + 1) * (prec + 1) + j + 1;
				indices[6 * (i * prec + j) + 5] = (i + 1) * (prec + 1) + j;
			}

		}
	
	
	
		return vertices;
	}
}
