#pragma once
#include "../Prereq.h"
#include "vulkan/vulkan.hpp"
namespace Voidstar
{

	template<typename T>
	const uint64_t SizeOfBuffer(const uint64_t bufferSize, const T& bufferElement)
	{
		return bufferSize * sizeof(bufferElement);
	}

	class Device;
	struct VOIDSTAR_API BufferInputChunk
	{
		size_t size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
	};

	class VOIDSTAR_API Buffer
	{
	public:
		Buffer(const BufferInputChunk& input);
		Buffer(const Buffer& buffer) = delete;
		static SPtr<Buffer> CreateStagingBuffer(size_t dataSize);

	


		void SetData(void* data,size_t size);
		vk::Buffer& GetBuffer() { return m_Buffer; }
		vk::DeviceMemory& GetMemory() { return m_BufferMemory; }
		uint32_t GetSize() { return m_Size; }
		void Free();
		~Buffer();	
	private:
		void AllocateBufferMemory(const BufferInputChunk& input, Device* device);
		friend class Renderer;
		friend class Device;
		uint32_t m_Size;
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
	};

}


