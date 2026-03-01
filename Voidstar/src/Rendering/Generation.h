#pragma once
#include <vector>
#include "Vertex_.h"
#include "IndexBuffer.h"
namespace Voidstar
{
	template<typename T>
	struct  QuadData
	{
		std::vector<T> verticies;
		std::vector<IndexType> indicies;
	};

	template<typename T>
	std::tuple<std::vector<T>, std::vector<IndexType>> GeneratePlane(float detail)
	{
		uint32_t seg = detail < 1 ? 1u : static_cast<uint32_t>(detail);
		uint32_t vx = seg + 1;
		uint32_t vy = seg + 1;

		std::vector<T> verticies;
		std::vector<IndexType> indicies;
		verticies.resize(vx * vy);
		indicies.reserve(seg * seg * 6);

		// XY plane, Z = 0, coords in [-1,1], UV in [0,1]
		for (uint32_t y = 0; y < vy; ++y) {
			float v = (float)y / (float)seg;
			float py = -1.0f + 2.0f * v;
			for (uint32_t x = 0; x < vx; ++x) {
				float u = (float)x / (float)seg;
				float px = -1.0f + 2.0f * u;

				uint32_t i = y * vx + x;
				verticies[i].Position[0] = px;
				verticies[i].Position[1] = py;
				verticies[i].Position[2] = 0.0f;
				verticies[i].UV[0] = u;
				verticies[i].UV[1] = v;
				// if T has Normal: set to (0,0,1)
				// out.verticies[i].Normal[0]=0; out.verticies[i].Normal[1]=0; out.verticies[i].Normal[2]=1;
			}
		}

		for (uint32_t y = 0; y < seg; ++y) {
			for (uint32_t x = 0; x < seg; ++x) {
				uint32_t i0 = y * vx + x;
				uint32_t i1 = y * vx + (x + 1);
				uint32_t i2 = (y + 1) * vx + (x + 1);
				uint32_t i3 = (y + 1) * vx + x;

				// two triangles (CCW)
				indicies.push_back(i0);
				indicies.push_back(i1);
				indicies.push_back(i2);
				indicies.push_back(i2);
				indicies.push_back(i3);
				indicies.push_back(i0);
			}
		}
		return { verticies, indicies};
	}
;
	inline float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }
	template<typename T>
	std::vector<T> GenerateSphere(float radius, float prec, std::vector<IndexType>& indices)
	{
		float numVertices = (prec + 1) * (prec + 1);
		float numIndices = prec * prec * 6;

		std::vector<T> vertices;
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
	template<typename T> 
	std::tuple<std::vector<T>,std::vector<IndexType>> GenerateCube()
	{
#if 0
		const std::vector<IndexType> indices =
		{
			0, 1, 3, 3, 1, 2,
			1, 5, 2, 2, 5, 6,
			5, 4, 6, 6, 4, 7,
			4, 0, 7, 7, 0, 3,
			3, 2, 7, 7, 2, 6,
			4, 5, 0, 0, 5, 1
		};

		std::vector<T> CubeVerticies;
		CubeVerticies.resize(8);
		//vertices[0].Position = { -1, -1, -1 };
		CubeVerticies[0].Position[0] = -1;
		CubeVerticies[0].Position[1] = -1;
		CubeVerticies[0].Position[2] = -1;


		//vertices[1].Position = { 1, -1, -1 };
		CubeVerticies[1].Position[0] = 1;
		CubeVerticies[1].Position[1] = -1;
		CubeVerticies[1].Position[2] = -1;
		//vertices[2].Position = { 1, 1, -1};
		CubeVerticies[2].Position[0] = 1;
		CubeVerticies[2].Position[1] = 1;
		CubeVerticies[2].Position[2] = -1;
		//vertices[3].Position = { -1, 1, -1 };
		CubeVerticies[3].Position[0] = -1;
		CubeVerticies[3].Position[1] = 1;
		CubeVerticies[3].Position[2] = -1;
		//vertices[4].Position = { -1, -1, 1 };
		CubeVerticies[4].Position[0] = -1;
		CubeVerticies[4].Position[1] = -1;
		CubeVerticies[4].Position[2] = 1;
		//vertices[5].Position = { 1, -1, 1};
		CubeVerticies[5].Position[0] = 1;
		CubeVerticies[5].Position[1] = -1;
		CubeVerticies[5].Position[2] = 1;
		//vertices[6].Position = { 1, 1, 1};
		CubeVerticies[6].Position[0] = 1;
		CubeVerticies[6].Position[1] = 1;
		CubeVerticies[6].Position[2] = 1;
		//vertices[7].Position = { -1, 1, 1 };
		CubeVerticies[7].Position[0] = -1;
		CubeVerticies[7].Position[1] = 1;
		CubeVerticies[7].Position[2] = 1;

		CubeVerticies[0].UV[0] = 0.0f; // U coordinate
		CubeVerticies[0].UV[1] = 0.0f; // V coordinate

		CubeVerticies[1].UV[0] = 1.0f;
		CubeVerticies[1].UV[1] = 0.0f;

		CubeVerticies[2].UV[0] = 1.0f;
		CubeVerticies[2].UV[1] = 1.0f;

		CubeVerticies[3].UV[0] = 0.0f;
		CubeVerticies[3].UV[1] = 1.0f;

		CubeVerticies[4].UV[0] = 0.0f;
		CubeVerticies[4].UV[1] = 0.0f;

		CubeVerticies[5].UV[0] = 1.0f;
		CubeVerticies[5].UV[1] = 0.0f;

		CubeVerticies[6].UV[0] = 1.0f;
		CubeVerticies[6].UV[1] = 1.0f;

		CubeVerticies[7].UV[0] = 0.0f;
		CubeVerticies[7].UV[1] = 1.0f;
		return { CubeVerticies ,indices };
#else
		std::vector<T> v(24);
	std::vector<IndexType> indices;
	indices.reserve(36);

	auto quad = [&](int faceIndex, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		int base = faceIndex * 4;
		v[base + 0].Position = { v0.x, v0.y, v0.z };
		v[base + 1].Position = { v1.x, v1.y, v1.z };
		v[base + 2].Position = { v2.x, v2.y, v2.z };
		v[base + 3].Position = { v3.x, v3.y, v3.z };

		// Per-face UVs (rectangle 0..1)
		v[base + 0].UV = { 0, 0 };
		v[base + 1].UV = { 1, 0 };
		v[base + 2].UV = { 1, 1 };
		v[base + 3].UV = { 0, 1 };

		// Two triangles
		indices.push_back(base + 0);
		indices.push_back(base + 1);
		indices.push_back(base + 2);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
		indices.push_back(base + 0);
		};

	// +X
	quad(0, { 1,-1,-1 }, { 1,-1, 1 }, { 1, 1, 1 }, { 1, 1,-1 });
	// -X
	quad(1, { -1,-1, 1 }, { -1,-1,-1 }, { -1, 1,-1 }, { -1, 1, 1 });
	// +Y
	quad(2, { -1, 1,-1 }, { 1, 1,-1 }, { 1, 1, 1 }, { -1, 1, 1 });
	// -Y
	quad(3, { -1,-1, 1 }, { 1,-1, 1 }, { 1,-1,-1 }, { -1,-1,-1 });
	// +Z
	quad(4, { -1,-1, 1 }, { -1, 1, 1 }, { 1, 1, 1 }, { 1,-1, 1 });
	// -Z
	quad(5, { 1,-1,-1 }, { 1, 1,-1 }, { -1, 1,-1 }, { -1,-1,-1 });

	return { v, indices };
#endif 
		
}


	

}