#include "Prereq.h"
#include "VertexBuffer.h"
#include "Device.h"
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

	void VertexBuffer::AllocateBufferMemory(const BufferInputChunk& input) {

		/*
		// Provided by VK_VERSION_1_0
		typedef struct VkMemoryRequirements {
			VkDeviceSize    size;
			VkDeviceSize    alignment;
			uint32_t        memoryTypeBits;
		} VkMemoryRequirements;
		*/
		auto& device = input.device->GetDevice();
		auto& devicePhys = input.device->GetDevicePhys();


		vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(m_Buffer);

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

		m_BufferMemory = device.allocateMemory(allocInfo);
		device.bindBufferMemory(m_Buffer, m_BufferMemory, 0);
	}

	void VertexBuffer::SetData(void* data)
	{
		void* memoryLocation = m_Device->GetDevice().mapMemory(m_BufferMemory, 0, m_Size);
		memcpy(memoryLocation, data, m_Size);
		m_Device->GetDevice().unmapMemory(m_BufferMemory);
	}


	VertexBuffer::VertexBuffer(const BufferInputChunk& input)
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.flags = vk::BufferCreateFlags();
		bufferInfo.size = input.size;
		bufferInfo.usage = input.usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		m_Device = input.device;
		m_Size = input.size;
		m_Buffer = input.device->GetDevice().createBuffer(bufferInfo);

		AllocateBufferMemory(input);
	}
	VertexBuffer::~VertexBuffer()
	{
		auto& device = m_Device->GetDevice();
		device.destroyBuffer(m_Buffer);
		device.freeMemory(m_BufferMemory);
	}
}