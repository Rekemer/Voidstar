#pragma once
#include "Vertex.h"
#include "../Prereq.h"
namespace Voidstar
{

	template<typename T>
	const uint64_t SizeOfBuffer(const uint64_t bufferSize, const T& bufferElement)
	{
		return bufferSize * sizeof(bufferElement);
	}

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
		Buffer(const Buffer& buffer) = delete;
		static SPtr<Buffer> CreateStagingBuffer(size_t dataSize);
		void AllocateBufferMemory(const BufferInputChunk& input, Device* device);
		void SetData(void* data);
		vk::Buffer& GetBuffer() { return m_Buffer; }
		vk::DeviceMemory& GetMemory() { return m_BufferMemory; }
		uint32_t GetSize() { return m_Size; }
		void Free();
		~Buffer();	
	private:
		friend class Renderer;
		friend class Device;
		uint32_t m_Size;
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
	};

}


