#pragma once
#include "Vertex.h"
#include "../Prereq.h"
namespace Voidstar
{
	class Device;
	struct BufferInputChunk 
	{
		size_t size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
	};

	class Buffer
	{
	public:
		Buffer(const BufferInputChunk& input);
		static SPtr<Buffer> CreateStagingBuffer(size_t dataSize);
		void AllocateBufferMemory(const BufferInputChunk& input, Device* device);
		void SetData(void* data);
		vk::Buffer& GetBuffer() { return m_Buffer; }
		vk::DeviceMemory& GetMemory() { return m_BufferMemory; }
		
		~Buffer();	
	private:
		uint32_t m_Size;
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
	};

}


