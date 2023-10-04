#include "Prereq.h"
#include "Buffer.h"
#include "Device.h"
#include "RenderContext.h"
namespace Voidstar
{
	uint32_t findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties) {

		/*
		* // Provided by VK_VERSION_1_0
		typedef struct VkPhysicalDeviceMemoryProperties {
			uint32_t        memoryTypeCount;
			VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
			uint32_t        memoryHeapCount;
			VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
		} VkPhysicalDeviceMemoryProperties;
		*/
		vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

			//bit i of supportedMemoryIndices is set if that memory type is supported by the device
			bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

			//propertyFlags holds all the memory properties supported by this memory type
			bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

			if (supported && sufficient) {
				return i;
			}
		}

		return 0;
	}

	void Buffer::AllocateBufferMemory(const BufferInputChunk& input, Device* device) {

		/*
		// Provided by VK_VERSION_1_0
		typedef struct VkMemoryRequirements {
			VkDeviceSize    size;
			VkDeviceSize    alignment;
			uint32_t        memoryTypeBits;
		} VkMemoryRequirements;
		*/
		auto& deviceLog = device->GetDevice();
		auto& devicePhys = device->GetDevicePhys();


		vk::MemoryRequirements memoryRequirements = deviceLog.getBufferMemoryRequirements(m_Buffer);

		/*
		* // Provided by VK_VERSION_1_0
		typedef struct VkMemoryAllocateInfo {
			VkStructureType    sType;
			const void*        pNext;
			VkDeviceSize       allocationSize;
			uint32_t           memoryTypeIndex;
		} VkMemoryAllocateInfo;
		*/
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryTypeIndex(
			devicePhys, memoryRequirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		m_BufferMemory = deviceLog.allocateMemory(allocInfo);
		deviceLog.bindBufferMemory(m_Buffer, m_BufferMemory, 0);
	}

	void Buffer::SetData(void* data)
	{
		auto device = RenderContext::GetDevice();
		void* memoryLocation = device->GetDevice().mapMemory(m_BufferMemory, 0, m_Size);
		memcpy(memoryLocation, data, m_Size);
		device->GetDevice().unmapMemory(m_BufferMemory);
	}


	Buffer::Buffer(const BufferInputChunk& input)
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.flags = vk::BufferCreateFlags();
		bufferInfo.size = input.size;
		bufferInfo.usage = input.usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		auto device = RenderContext::GetDevice();
		m_Size = input.size;
		m_Buffer = device->GetDevice().createBuffer(bufferInfo);

		AllocateBufferMemory(input,device);
	}
	SPtr<Buffer> Buffer::CreateStagingBuffer(size_t dataSize)
	{
		BufferInputChunk inputBuffer;
		inputBuffer.size = dataSize;
		inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		inputBuffer.usage = vk::BufferUsageFlagBits::eTransferSrc;

		SPtr<Buffer> buffer =CreateSPtr<Buffer>(inputBuffer);
		return buffer;
	}

	void Buffer::Free()
	{
		auto& device = RenderContext::GetDevice()->GetDevice();
		device.destroyBuffer(m_Buffer);
		device.freeMemory(m_BufferMemory);

	}
	Buffer::~Buffer()
	{
		Free();
	}
}