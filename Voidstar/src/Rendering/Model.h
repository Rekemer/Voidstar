#pragma once
#include "Types.h"
#include "Vertex.h"
#include "vulkan/vulkan.hpp"
#include "../Prereq.h"
namespace Voidstar
{
	class Buffer;
	typedef uint32_t IndexType;
	class Model
	{
	public:
		static SPtr<Model> Load(std::string& pathGeometry);
	private:
		friend class Renderer;
		std::vector<Vertex> vertices;
		std::vector<IndexType> indices;
		UPtr<Buffer> vertexBuffer;
		
	};
}