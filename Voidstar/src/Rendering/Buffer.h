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
		vk::MemoryPropertyFlags memoryProperties;
	};

	class Buffer
	{
	public:
		Buffer(const BufferInputChunk& input);
		void AllocateBufferMemory(const BufferInputChunk& input, Device* device);
		void SetData(void* data);
		vk::Buffer& GetBuffer() { return m_Buffer; }
		vk::DeviceMemory& GetMemory() { return m_BufferMemory; }
		void Bind() const;
		void Unbind() const;

		~Buffer();	
	private:
		uint32_t m_Size;
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
	};

}


