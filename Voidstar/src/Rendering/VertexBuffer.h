#pragma once
#include "Vertex.h"
#include "../Prereq.h"
namespace Voidstar
{
	class Device;
	struct BufferInputChunk 
	{
		size_t size;
		vk::BufferUsageFlagBits usage;
		Device* device;
		vk::MemoryPropertyFlags memoryProperties;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(const BufferInputChunk& input);
		void AllocateBufferMemory(const BufferInputChunk& input);
		void SetData(void* data);
		vk::Buffer& GetBuffer() { return m_Buffer; }
		void Bind() const;
		void Unbind() const;

		~VertexBuffer();	
	private:
		uint32_t m_Size;
		Device* m_Device;
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
	};

}


