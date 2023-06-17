#pragma once
#include "vulkan/vulkan.hpp"
namespace Voidstar
{
	class Instance;
	class Device
	{
	public:
		static Device* Create(Instance* intsance , vk::SurfaceKHR surface);
		vk::SampleCountFlagBits GetMaxUsableSampleCount();
		vk::Device& GetDevice() { return m_Device; }
		vk::PhysicalDevice& GetDevicePhys() { return m_PhysicalDevice; }
		vk::Queue& GetGraphicsQueue() { return m_GraphicsQueue; }
		size_t GetGraphicsIndex() { return m_GraphicFamily; }
		vk::Queue& GetPresentQueue() { return m_PresentQueue;}
		size_t GetPresentsIndex() { return m_PresentFamily; }
		uint32_t FindMemoryTypeIndex (uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
		vk::SampleCountFlagBits GetSamples() { return m_MsaaSamples; }
	private:
		vk::PhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		vk::Device m_Device = VK_NULL_HANDLE;
		vk::SampleCountFlagBits m_MsaaSamples;
		size_t m_GraphicFamily{ 0 }, m_PresentFamily{ 0 };
		vk::Queue m_GraphicsQueue;
		vk::Queue m_PresentQueue;
	};
}

