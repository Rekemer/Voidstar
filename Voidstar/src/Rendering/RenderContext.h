#pragma once
#include"Prereq.h"
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	class Device;
	class CommandBuffer;
	struct Attachment;
	struct Swapchain;
	class Window;
	class InstanceInfo;
	class Instance;
	class SwapChainSupportDetails;
	class RenderContext
	{
	public:
		static Device* GetDevice();
		static vk::SurfaceKHR* GetSurface() { return m_Surface.get(); };
		static void SetDevice(Device* device);
		static void CreateSwapchain(SwapChainSupportDetails& attachment);
		static void RecreateSwapchain(SwapChainSupportDetails& attachment);
		static void CreateSurface(Window* window);
		static void CreateInstance(InstanceInfo& window);
		static void CreateDevice();
		static void Shutdown();
		static Swapchain* GetSwapchain() { return m_Swapchain.get(); };

	private:
		static Device* m_Device;
		static UPtr<Swapchain> m_Swapchain;
		static UPtr<Instance> m_Instance;
		static UPtr <vk::SurfaceKHR> m_Surface;
	};
}