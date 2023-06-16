#include "Prereq.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Model.h"
#include "Log.h"
#include "Buffer.h"
#include "glm.hpp"
#include "gtx/rotate_vector.hpp"
namespace Voidstar
{
	SPtr<Model> Voidstar::Model::Load(std::string& pathGeometry)
	{
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        auto model = CreateSPtr <Model>();

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathGeometry.c_str()))
        {
            Log::GetLog()->error("path {0} not found", pathGeometry);
        }

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                     
                vertex.Position[0] = attrib.vertices[3 * index.vertex_index + 0];
                vertex.Position[1] = attrib.vertices[3 * index.vertex_index + 1];
                vertex.Position[2] = attrib.vertices[3 * index.vertex_index + 2];
           
                vertex.UV[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.UV[1] = attrib.texcoords[2 * index.texcoord_index + 1];
                vertex.Color[0] = 1.0;
                vertex.Color[1] = 1.0;
                vertex.Color[2] = 1.0;
                vertex.Color[3] = 1.0;

                model->vertices.push_back(vertex);
                model->indices.push_back(model->indices.size());
            };

            
        }
    

      
		return model;
	}
}